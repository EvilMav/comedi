

AC_DEFUN(DS_LINUX_DIR,
  [AC_ARG_WITH([linuxdir],
    [AC_HELP_STRING([--with-linuxdir],
      [specify path to Linux source directory])],
    [LINUX_DIR="${withval}"],
    [LINUX_DIR=default])

  if test "${LINUX_DIR}" != "default" ; then
    DS_TRY_LINUX_DIR(${LINUX_DIR}, , AC_MSG_ERROR("not found") )
  fi

  if test "${LINUX_DIR}" = "default" ; then
    dir="/lib/modules/`uname -r`/build";
    DS_TRY_LINUX_DIR(${dir}, LINUX_DIR=${dir}, )
  fi
  if test "${LINUX_DIR}" = "default" ; then
    dir="../linux";
    DS_TRY_LINUX_DIR(${dir}, LINUX_DIR=${dir}, )
  fi
  if test "${LINUX_DIR}" = "default" ; then
    dir="/usr/src/linux";
    DS_TRY_LINUX_DIR(${dir}, LINUX_DIR=${dir}, )
  fi

  if test "${LINUX_DIR}" = "default" ; then
    AC_MSG_ERROR(Linux source directory not found)
  fi

  AC_SUBST(LINUX_DIR)
])

AC_DEFUN(DS_TRY_LINUX_DIR,
  [AC_MSG_CHECKING(for Linux in $1)

  if test -f "$1/Makefile" ; then
    result=yes
    $2
  else
    result="not found"
    $3
  fi

  AC_MSG_RESULT($result)
])

AC_DEFUN(DS_LINUX_2_6,
[
  DS_LINUX_DIR()
  AC_MSG_CHECKING(for Linux CFLAGS)

  tmpdir="`pwd`/tmp-noicrwa"

  rm -rf ${tmpdir}
  mkdir ${tmpdir}
  
  cat >${tmpdir}/Makefile <<EOF
obj-m += fake.o

\$(obj)/fake.c: flags
	touch \$(obj)/fake.c

.PHONY: flags
flags:
	echo LINUX_ARCH=\"\$(ARCH)\" >>\$(obj)/flags
	echo LINUX_AFLAGS=\"\$(AFLAGS)\" | sed 's_Iinclude_I"\$(LINUXDIR)/include"_g'>>\$(obj)/flags
	echo LINUX_LDFLAGS=\"\" >>\$(obj)/flags
	echo LINUX_ARFLAGS=\"\$(ARFLAGS)\" >>\$(obj)/flags
	echo LINUX_CROSS_COMPILE=\"\$(CROSS_COMPILE)\" >>\$(obj)/flags
	echo LINUX_KERNELRELEASE=\"\$(KERNELRELEASE)\" >>\$(obj)/flags
	echo LINUX_CFLAGS=\"\$(CFLAGS)\" | sed 's_Iinclude_I"\$(LINUXDIR)/include"_g'>>\$(obj)/flags
	echo LINUX_CC=\"\$(CC)\" >>\$(obj)/flags
	echo LINUX_LD=\"\$(LD) \$(LDFLAGS)\" >>\$(obj)/flags
	echo LINUX_AS=\"\$(AS)\" >>\$(obj)/flags
EOF

  echo make -C ${LINUX_DIR} V=1 SUBDIRS=${tmpdir} LINUXDIR=${LINUX_DIR} MODVERDIR=${tmpdir} modules >&5 2>&5
  make -C ${LINUX_DIR} V=1 SUBDIRS=${tmpdir} LINUXDIR=${LINUX_DIR} MODVERDIR=${tmpdir} modules >&5 2>&5
  . ${tmpdir}/flags
  rm -rf ${tmpdir}

  LINUX_MODULE_EXT=".ko"

  AC_SUBST(LINUX_ARCH)
  AC_SUBST(LINUX_AFLAGS)
  AC_SUBST(LINUX_LDFLAGS)
  AC_SUBST(LINUX_ARFLAGS)
  AC_SUBST(LINUX_CROSS_COMPILE)
  AC_SUBST(LINUX_KERNELRELEASE)
  AC_SUBST(LINUX_CFLAGS)
  AC_SUBST(LINUX_CC)
  AC_SUBST(LINUX_LD)
  AC_SUBST(LINUX_AS)
  AC_SUBST(LINUX_MODULE_EXT)

  AC_MSG_RESULT([ok])
])


AC_DEFUN(DS_LINUX_2_4,
[
  DS_LINUX_DIR()
  AC_MSG_CHECKING(for Linux CFLAGS)

  tmpdir="`pwd`/tmp-noicrwa"

  rm -rf ${tmpdir}
  mkdir ${tmpdir}
  
  cat >${tmpdir}/Makefile <<EOF

.PHONY: modules
modules:
	echo LINUX_ARCH=\"\$(ARCH)\" >>flags
	echo LINUX_AFLAGS=\"\$(AFLAGS)\" | sed 's_Iinclude_I\"\$(LINUXDIR)/include\"_g'>>flags
	echo LINUX_LDFLAGS=\"\" >>flags
	echo LINUX_ARFLAGS=\"\$(ARFLAGS)\" >>flags
	echo LINUX_CROSS_COMPILE=\"\$(CROSS_COMPILE)\" >>flags
	echo LINUX_KERNELRELEASE=\"\$(KERNELRELEASE)\" >>flags
	echo LINUX_CFLAGS=\"\$(CFLAGS)\" | sed 's_Iinclude_I\"\$(LINUXDIR)/include\"_g'>>flags
	echo LINUX_CC=\"\$(CC)\" >>flags
	echo LINUX_LD=\"\$(LD) \$(LDFLAGS)\" >>flags
	echo LINUX_AS=\"\$(AS)\" >>flags
EOF

  echo make -C ${LINUX_DIR} SUBDIRS=${tmpdir} modules >&5 2>&5
  make -C ${LINUX_DIR} SUBDIRS=${tmpdir} modules >&5 2>&5
  . ${tmpdir}/flags
  rm -rf ${tmpdir}

  LINUX_MODULE_EXT=".o"

  AC_SUBST(LINUX_ARCH)
  AC_SUBST(LINUX_AFLAGS)
  AC_SUBST(LINUX_LDFLAGS)
  AC_SUBST(LINUX_ARFLAGS)
  AC_SUBST(LINUX_CROSS_COMPILE)
  AC_SUBST(LINUX_KERNELRELEASE)
  AC_SUBST(LINUX_CFLAGS)
  AC_SUBST(LINUX_CC)
  AC_SUBST(LINUX_LD)
  AC_SUBST(LINUX_AS)
  AC_SUBST(LINUX_MODULE_EXT)

  AC_MSG_RESULT([ok])
])

AC_DEFUN(DS_CHECK_LINUX_CONFIG_OPTION,
[
  AC_MSG_CHECKING([Linux config option $1])

  if grep '^$1=y$' ${LINUX_DIR}/.config >/dev/null 2>/dev/null; then
    result=yes
    $2
  else if grep '^$1=m$' ${LINUX_DIR}/.config >/dev/null 2>/dev/null; then
    result=module
    $3
  else
    result=no
    $4
  fi
  fi

  AC_MSG_RESULT([$result])
])

AC_DEFUN(DS_LINUX_CONFIG_OPTION,
[
  DS_CHECK_LINUX_CONFIG_OPTION([$1],
    [$1=yes],
    [$1=module],
    [$1=no])

  AM_CONDITIONAL([$1],[test "${$1}" = yes])
])

AC_DEFUN(DS_LINUX_CONFIG_OPTION_MODULE,
[
  DS_CHECK_LINUX_CONFIG_OPTION([$1],
    [$1=yes],
    [$1=module],
    [$1=no])

  AM_CONDITIONAL([$1],[test "${$1}" = yes -o "${$1}" = module])
])
