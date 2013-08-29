#!/bin/bash

rm -f mazacat-lin.tar.gz
rm -rf mazacat-lin

mkdir mazacat-lin
svn export . EXPORTED
cp -R EXPORTED/src mazacat-lin
cp EXPORTED/{aclocal.m4,ChangeLog,config*,depcomp,install-sh,LGPL,Makefile*,missing,README} mazacat-lin
rm -rf EXPORTED
tar cvzf mazacat-lin.tar.gz mazacat-lin
rm -rf mazacat-lin

