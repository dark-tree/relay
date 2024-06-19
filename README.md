## URP Server
This is an reference implementation of the User Relay Protocol, written in C

#### Build
To build the project run:
```sh
mkdir build
cmake . -B build
cmake --build build/

# start the relay
./build/server

# start the debug client
./build/client localhost
```

#### License
The code of this reference server implementation is licensed under the GPLv3 license, see [here](LICENSE).
While the included client libraries (file under `/lib`) are licensed under LGPL. See relevant file headers.
