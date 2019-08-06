#!/bin/bash

export BCDS_DEVELOPMENT_TOOLS
export PATH=${BCDS_DEVELOPMENT_TOOLS}/Doxygen/:${BCDS_DEVELOPMENT_TOOLS}/Graphviz/v2.38-1/bin/:${PATH}

echo Building doxygen
cd ../docs && ${BCDS_DEVELOPMENT_TOOLS}/Doxygen/v1.8.8_32Bit/doxygen.exe > doxygenDump.txt
cd ./html
${BCDS_DEVELOPMENT_TOOLS}/Doxygen/v1.8.8_32Bit/hhc.exe index.hhp
cd ..
echo Documentation 
exit
