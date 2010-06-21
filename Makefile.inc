SRC_DIR   = src
LIB_DIR   = lib
BUILD_DIR = build
DIST_DIR  = dist

MAKE      = make
ZIP       = zip -qr

RHINO     = java -jar lib/common.make/js.jar
COMPRESS  = java -jar lib/common.make/yuicompressor-2.4.2.jar --type js

VERSION = $(shell git describe --tags | cut -d'-' -f1,2)

DIST          = ${APP}-${VERSION}.zip
DIST_SRCS     = LICENSE README ${TARGETS} examples

DIST-SRC      = ${APP}-${VERSION}-src.zip
DIST-SRC_SRCS = LICENSE README Makefile ${SRC_DIR} ${LIB_DIR} examples

ifeq (${BUILD_STYLE}, simple)
	TARGETS = ${BUILD_DIR}/${APP}.min.js
	EXAMPLE_TARGET = ${APP}.min.js
else
	TARGETS = ${BUILD_DIR}/${APP}.shared.min.js \
	          ${BUILD_DIR}/${APP}.standalone.min.js;
	EXAMPLE_TARGET = ${APP}.standalone.min.js
endif

DIST_TARGETS = ${DIST_DIR}/${DIST} \
			         ${DIST_DIR}/${DIST-SRC} \
							 ${MORE_DIST_TARGETS}

STANDALONE_SRCS = ${SRCS}
SHARED_SRCS     = ${SHARED_EXTRA_SRCS} ${SRCS}

# TO CONFIGURE:
SRCS               =
SHARED_EXTRA_SRCS  =

LIBS               =
UPDATE_LIBS        = 

MORE_TARGETS       = 
MORE_DIST_TARGETS  = 

MORE_DIST-SRCS     =
MORE_DIST-SRC_SRCS = 

all: prepare ${TARGETS}

dist: prepare ${DIST_TARGETS}

prepare: init check-deps update 

init:
	@git submodule init
	@git submodule update

check-deps:
	@which zip  >/dev/null || ( echo "ERROR: missing : zip";  exit 1 )
	@which git  >/dev/null || ( echo "ERROR: missing : git";  exit 1 )
	@which java >/dev/null || ( echo "ERROR: missing : java"; exit 1 )

update: ${UPDATE_LIBS}
	@for l in ${UPDATE_LIBS}; do \
    ( cd $$l; ${MAKE}; ) \
  done

${BUILD_DIR}:
	@mkdir  -p ${BUILD_DIR}

lint: ${SRCS}
	@echo "*** checking source correctness using JSLint"
	@for f in ${SRCS}; do \
	  ${RHINO} lib/common.make/jslint-check.js $$f; \
	done

${BUILD_DIR}/${APP}.shared.js: ${BUILD_DIR} ${SHARED_SRCS}
	@echo "*** building $@"
	@cat ${SHARED_SRCS} > $@
	@echo "\n${APP}.version = \"${VERSION}\";\n" >> $@;

${BUILD_DIR}/${APP}.js: ${BUILD_DIR} ${SHARED_SRCS}
	@echo "*** building $@"
	@cat ${SHARED_SRCS} > $@
	@echo "\n${APP}.version = \"${VERSION}\";\n" >> $@;

${BUILD_DIR}/${APP}.standalone.js: ${BUILD_DIR} ${LIBS} ${STANDALONE_SRCS}
	@echo "*** building $@"
	@cat ${LIBS} ${STANDALONE_SRCS} > $@
	@echo "\n${APP}.version = \"${VERSION}\";\n" >> $@;

${DIST_DIR}/${DIST}: ${DIST_SRCS}
	@echo "*** packaging ${APP} distribution"
	@mkdir -p ${DIST_DIR}/js/${APP}
	@cp -r ${DIST_SRCS} ${DIST_DIR}/js/${APP}/
	@( cd ${DIST_DIR}/js/${APP}/examples; \
	   for f in *.html; do \
		cat $$f \
		| sed -e 's/includes\.js/..\/${APP}.standalone.min.js/' \
		> $$f.new; \
		mv $$f.new $$f; \
	done )
	@(cd ${DIST_DIR}/js; ${ZIP} ../${DIST} ${APP})

${DIST_DIR}/${DIST-SRC}: ${DIST-SRC_SRCS}
	@echo "*** packaging ${APP} src distribution"
	@mkdir -p ${DIST_DIR}/src/${APP}
	@cp -r ${DIST-SRC_SRCS} ${DIST_DIR}/src/${APP}
	@(cd ${DIST_DIR}/src; ${ZIP} ../${DIST-SRC} ${APP})

%.min.js: %.js
	@echo "*** building $@"
	@${COMPRESS} $< > $@

clean:
	@find . | grep "~$$" | xargs rm -f
	@rm -rf ${DIST_DIR}

mrproper: clean
	@rm -rf ${BUILD_DIR}
