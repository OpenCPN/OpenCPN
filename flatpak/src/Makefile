all:
	echo "Nothing to build"

install:
	#  Make the plugins solibs available at 
	#  /app/extensions/Base/lib/opencpn
	#
	install -d /app/extensions/Base/lib/opencpn/plugins
	cp -ar /app/lib/opencpn.ORG/* /app/extensions/Base/lib/opencpn
	#
	# Make all plugin data available at 
	# /app/extensions/Base/share/opencpn/plugins
	#
	install -d /app/extensions/Base/share/opencpn/plugins
	cp -ar /app/share/opencpn/plugins.ORG/* \
		/app/extensions/Base/share/opencpn/plugins
	#
	# Messy: make all existing message files available at
	# /app/extensions/share/locale, which TEXTDOMAINDIR points
	# to in runtime.
	#
	install -d /app/extensions/Base/share/locale
	for f in /app/share/runtime/locale/*/share/*/LC_MESSAGES/*.mo; do \
	    dest=$$(echo $$f | sed 's|/app/share.*share|/app/extensions/Base/share/locale|'); \
	    destdir=$$(dirname $$dest); \
	    test -d $$destdir || mkdir -p $$destdir; \
	    ln -sf $$f $$dest; \
	done
	for f in /usr/share/locale/*/LC_MESSAGES/*.mo; do \
	    dest=$$(echo $$f | sed 's|/usr|/app/extensions/Base|'); \
	    destdir=$$(dirname $$dest); \
	    test -d $$destdir || mkdir -p $$destdir; \
	    ln -sf $$f $$dest; \
	done
	for f in $$(find /app/share/runtime/locale/*/share/* -name *mo); do \
	    dest=$$(dirname $$f)/../../../LC_MESSAGES; \
	    dest=$$(echo $$dest | sed 's|/app/share/runtime|/app/extensions/Base/share|'); \
	    test -d $$dest || mkdir -p $$dest; \
	    ln -sf $$f $$dest; \
	done


