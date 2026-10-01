#!/bin/bash

for i in $(seq 1 1000); do
    ./tdescriptor huellasxyt/huella${i}.xyt huellasbin/huella${i}.bin -N 8 -C LSSR
done
