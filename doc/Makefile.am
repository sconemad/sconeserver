# SconeServer (http://www.sconemad.com)
# automake file for sconeserver documentation

SUBDIRS = examples html

EXTRA_DIST = sitedef src tpl

dist-hook:
	rm -rf `find $(distdir)/src -name CVS`
	rm -rf `find $(distdir)/tpl -name CVS`

