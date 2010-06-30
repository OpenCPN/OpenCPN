#convert to png
inkscape --without-gui --export-png=down.png --export-dpi=72 --export-background-opacity=0 --export-width=64 --export-height=64 down.svg &> /dev/null
inkscape --without-gui --export-png=left.png --export-dpi=72 --export-background-opacity=0 --export-width=64 --export-height=64 left.svg &> /dev/null
inkscape --without-gui --export-png=up.png --export-dpi=72 --export-background-opacity=0 --export-width=64 --export-height=64 up.svg &> /dev/null
inkscape --without-gui --export-png=right.png --export-dpi=72 --export-background-opacity=0 --export-width=64 --export-height=64 right.svg &> /dev/null
inkscape --without-gui --export-png=pencil.png --export-dpi=72 --export-background-opacity=0 --export-width=64 --export-height=64 pencil.svg &> /dev/null
inkscape --without-gui --export-png=mob.png --export-dpi=72 --export-background-opacity=0 --export-width=32 --export-height=32 mob.svg &> /dev/null

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
  #rm $pic
done
echo "Finished!"

