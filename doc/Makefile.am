# SconeServer (http://www.sconemad.com)
# automake file for sconeserver documentation

SUBDIRS = examples

EXTRA_DIST = 

# Remove subversion dirs when we pull in whole subdirectories
dist-hook:
	rm -rf `find $(distdir)/src -name .svn`
	rm -rf `find $(distdir)/tpl -name .svn`

