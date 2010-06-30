#convert to png
for pic in $(ls *.svg)
do
  echo "converting $pic"
  inkscape --without-gui --export-png=$(basename $pic .svg ).png --export-dpi=72 --export-background=rgb\(223,223,223\) --export-width=16 --export-height=16 $pic &> /dev/null
done

#convert to xpm
for pic in $(ls *.png)
do
  echo "converting $pic"
  convert $pic $(basename $pic .png ).xpm
done

#change static char to static const char
echo "Conversion finished, now fixing convert errors"
for pic in $(ls *.xpm)
do
  echo "fixing $pic"
  sed 's/static char/static const char/' <$pic> post_$pic
done

#strip first post_ characters
for F in post_*
do
  mv $F ${F:5}
done

#move files
echo "Fix finished, now moving to xpm directory"
for pic in $(ls *.xpm)
do
  echo "moving $pic"
  mv $pic ../
done

#clean up
echo "Cleaning up"
for pic in $(ls *.png)
do
  echo "cleaning $pic"
  rm $pic
done
echo "Finished!"

