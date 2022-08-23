#!/bin/bash

for i in *.vert; do
    filename="${i%.*}"
    glslangValidator.exe -V $i -o $filename.vert.spv
done

for i in *.frag; do
    filename="${i%.*}"
    glslangValidator.exe -V $i  -o $filename.frag.spv
done