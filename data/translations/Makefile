ILMENDUR_SOURCE_FILES = $(shell find ../../src -iregex '.*\.[ch]pp$$' -print0 | xargs -0)

potfile:
	xgettext \
		--output=Ilmendur.pot \
		--msgid-bugs-address=kontakt@guelker.eu \
		--add-comments="TRANS:" \
		--language=C++ \
		--escape \
		--from-code=UTF-8 \
		--keyword=_ \
		--keyword=N_ \
		--copyright-holder="Marvin Gülker and the Ilmendur team" \
		--package-name="Ilmendur" \
		$(ILMENDUR_SOURCE_FILES)

%.po: Ilmendur.pot
	msgmerge -U $@ $<

.PHONY: potfile
