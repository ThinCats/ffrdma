#!/bin/bash

./build/test/hello_world --r_myip 10.10.0.112 --r_myport 55001 --r_hostmap 10.10.0.112:55000+55001,10.10.0.110:55000+55001
