name: Platform Compatibility Testing

on:
  push:
    branches: [ "main" ]
  pull_request:
    branches: [ "main" ]

jobs:
  build:
    name: 'Conduct compatibility testing on different platforms'
    strategy:
      matrix:
        os: [windows-latest, ubuntu-latest]
        compiler: [msvc, gcc, clang]
        exclude:
          - os: windows-latest
            compiler: gcc
          - os: windows-latest
            compiler: clang
          - os: ubuntu-latest
            compiler: msvc

    runs-on: ${{ matrix.os }}

    steps:
    - name: Checkout code
    - uses: actions/checkout@v4

    # Windows
    - name: Setup windows compiler ( msvc )
      if: runner.os == 'Windows'
      shell: powershell
      run: |
        Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
        choco install cmake -y

        git clone --depth=1 https://github.com/Microsoft/vcpkg.git
        cd vcpkg
        .\scripts\bootstrap.ps1
        echo "VCPKG_ROOT=${pwd}" >> $env:GITHUB_ENV
        echo "VCPKG_DEFAULT_TRIPLET=x64-windows" >> $env:GITHUB_ENV

        vcpkg install openssl
        vcpkg install zlib

    # Linux
    - name: Setup linux compiler ( gcc / clang )
      if: runner.os == 'Linux'
      run: |
        sudo apt-get update
        sudo apt-get upgrade
        sudo apt-get install
        sudo apt-get install -y gcc g++ make cmake
        git clone --depth=1 https://github.com/Microsoft/vcpkg.git
        cd vcpkg
        ./bootstrap-vcpkg.sh

        vcpkg install openssl
        vcpkg install zlib
