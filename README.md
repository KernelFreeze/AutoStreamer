# Installation
```bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
sudo apt-get update
sudo apt-get install -y clang-6.0
sudo apt-get install -y rtmpdump
```

# Build
```bash
tools/linux/genie gmake
make
```

# Run
```bash
bin/debug/streamer
```