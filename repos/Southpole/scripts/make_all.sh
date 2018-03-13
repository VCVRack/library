!/bin/sh
make clean
make -j8 dist
rm -rf ../Southpole
cp -r dist/Southpole ..
make clean
make -j8 -f Makefile.parasites dist
rm -rf ../Southpole-parasites
cp -r dist/Southpole-parasites ..
make clean
