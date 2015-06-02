#!/usr/bin/env bash

mkdir $1;
cat >$1/$1.c <<EOF
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <helpers.h>
#include <bufio.h>

int main(int argc, char** argv)
{
  return 0;
}
EOF
sed "s/filesender/$1/g" filesender/Makefile >$1/Makefile
sed "s/filesender/$1/g" filesender/.gitignore >$1/.gitignore
