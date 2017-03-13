#!/bin/bash

#Optimize the JPG images
JPGS=`find . -name "*.jp*"`
for fj in $JPGS
do

echo $fj

/opt/mozjpeg/bin/cjpeg -outfile ${fj}.new ${fj}
mv ${fj}.new ${fj}

done


#Optimize the PNG images
PNGS=`find . -name "*.png"`
for fp in $PNGS
do
    echo $fp

#    gm convert ${fp} -colors 32 ${fp}
#    optipng -o7 ${fp}
done

find . -name '*.png' -exec pngquant --ext .png --force 256 {} \;


