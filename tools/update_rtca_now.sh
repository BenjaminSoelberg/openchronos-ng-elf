#!/bin/bash

cat << EOF > drivers/rtca_now.h

#ifndef __RTCA_NOW_H__
#define __RTCA_NOW_H__

#define COMPILE_YEAR `date +%Y`
#define COMPILE_MON `date +%m`
#define COMPILE_DAY `date +%d`
#define COMPILE_DOW `date +%u`
#define COMPILE_HOUR `date +%H`
#define COMPILE_MIN `date +%M`

#endif
EOF
