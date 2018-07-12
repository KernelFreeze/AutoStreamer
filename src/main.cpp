#include <filesystem>
#include <fstream>
#include <iostream>
#include <librtmp/rtmp.h>
#include <string>

#include "nanolog.h"

#include "INIReader.h"
#include "flv/flv.h"

namespace fs = std::filesystem;
using namespace std;

#define BUFFER_SIZE 10000000
#define CONFIG "config.ini"
#define VIDEO_PATH "videos"

inline bool exists(const char *fileName) {
    ifstream infile(fileName);
    return infile.good();
}

void writeConfig() {
    LOG_INFO << "Generating config file";
    ofstream config(CONFIG, ios::out);

    if (!config.is_open()) {
        LOG_CRIT << "Unable to open config file to write.";
        return;
    }

    config << "[stream]" << endl
           << "version=1" << endl
           << "token=MIXERTOKEN" << endl
           << "url=rtmp://ingest-par.mixer.com:1935/beam"
           << " ; https://watchbeam.zendesk.com/hc/en-us/articles/"
              "209659883-How-to-change-your-Ingest-Server"
           << endl;
    config.close();
}

int main(int argc, char const *argv[]) {
    nanolog::initialize(nanolog::GuaranteedLogger(), "log/", "stream", 1);

    LOG_INFO << "Starting GOTH streamer";

    if (!exists(CONFIG)) {
        writeConfig();

        LOG_INFO << "The configuration file was generated, please add your mixer access "
                 << "token and stream URL.";
        return 0;
    }

    INIReader reader(CONFIG);

    string rtmp_url = reader.Get("stream", "url", "rtmp://ingest-par.mixer.com:1935/beam");
    rtmp_url.append(" live=1");
    rtmp_url.append(" token=");
    rtmp_url.append(reader.Get("stream", "token", "UNKOWN"));
    rtmp_url.append(" timeout=20");

    RTMP *rtmp = RTMP_Alloc();

    if (!rtmp) {
        LOG_CRIT << "Unable to create rtmp object";
        return 1;
    }

    RTMP_Init(rtmp);
    RTMP_SetupURL(rtmp, const_cast<char *>(rtmp_url.c_str()));
    RTMP_EnableWrite(rtmp);

    if (!RTMP_Connect(rtmp, NULL)) {
        LOG_CRIT << "Unable to connect to server";
        RTMP_Free(rtmp);
        return 1;
    }

    if (!RTMP_ConnectStream(rtmp, 0)) {
        LOG_CRIT << "Unable to connect to stream";
        RTMP_Free(rtmp);
        return 1;
    }

    for (auto &file : fs::directory_iterator(VIDEO_PATH)) {
        auto input_file = file.path().c_str();

        // Load FLV file
        auto flvin = flv_open(input_file);

        if (!flvin) {
            LOG_CRIT << "Unable to open " << input_file;
            RTMP_Free(rtmp);
            return 1;
        }

        flv_header header;
        int res = flv_read_header(flvin, &header);
        if (res == FLV_ERROR_NO_FLV || res == FLV_ERROR_EOF) {
            LOG_CRIT << "Input file is not an FLV video";
            RTMP_Free(rtmp);
            flv_close(flvin);
            return 1;
        }

        auto buffer = new char[BUFFER_SIZE];

        // Decode FLV data

        flv_tag tag;
        while (flv_read_tag(flvin, &tag) != FLV_ERROR_EOF) {
            // copy tag header
            flv_copy_tag(buffer, &tag, FLV_TAG_SIZE);

            // copy tag body
            size_t data_size =
                flv_read_tag_body(flvin, buffer + FLV_TAG_SIZE, BUFFER_SIZE - FLV_TAG_SIZE);

            // copy previous tag size
            uint32 pts;
            flv_read_prev_tag_size(flvin, &pts);
            flv_copy_prev_tag_size(buffer + FLV_TAG_SIZE + flv_tag_get_body_length(tag), pts,
                BUFFER_SIZE - (FLV_TAG_SIZE + flv_tag_get_body_length(tag)));

            // write the packet
            int size = FLV_TAG_SIZE + data_size + sizeof(uint32);

            if (RTMP_Write(rtmp, buffer, size) <= 0) {
                LOG_CRIT << "Unable to write bytes to server";
                break;
            }
        }

        flv_close(flvin);
        delete[] buffer;
    }

    RTMP_Free(rtmp);
    return 0;
}