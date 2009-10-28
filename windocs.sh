cat docs/docs_template.conf | sed \
 -e "s/XXNITRO_VERSIONXX/`win32/VerRelease/nitro_version`/" \
 -e "s/XXBUILD_DIRXX/docs/" > docs/docs.conf
doxygen docs/docs.conf
rm docs/docs.conf
