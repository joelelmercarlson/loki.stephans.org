#
# @(#) $Id: Makefile,v 1.19 2003/03/03 17:16:34 loki Exp $
#
# make install to setup/maintain your LokiSa environment
#
SHELL=/bin/sh
PREFIX=${HOME}
LOKISA=${PREFIX}/.lokisa
LOKISA_BIN=${LOKISA}/bin
PERL=perl
CP=/bin/cp -fp
MKDIR=/bin/mkdir
ECHO=/bin/echo

TARGETS	= sa-dbm.pl sa-xml.pl sa-filter.pl LokiSa.pm spamgrep.pl

S=${LOKISA}/mail/spam
G=${LOKISA}/mail/good
W=${LOKISA}/mail/white

help:
	@${ECHO} "'make install' will result in:"
	@${ECHO} "    Creating ${HOME}/.lokisa/ for the databases good, spam, and white"
	@${ECHO} "    Installing sa-dbm.pl, sa-filter.pl, and LokiSa.pm into ${LOKISA}"
	@${ECHO} "    Create sa-procmail to be copied to ${HOME}/.procmailrc"
	@${ECHO}
	@${ECHO} "'make dbs' will make good, spam, and white DBs from"
	@${ECHO} "    S=${S}"
	@${ECHO} "    G=${G}"
	@${ECHO} "    W=${W}"
	@${ECHO}
	@${ECHO} "Additional targets:"
	@${ECHO} "  To create empty DBs:"
	@${ECHO} "    clean_dbs"
	@${ECHO} "  To create separately:"
	@${ECHO} "    good_db, spam_db, white_db"
	@${ECHO} "  To read:"
	@${ECHO} "    read_good, read_spam, read_white, read_bayes"
	@${ECHO} "  To test, i.e.:"
	@${ECHO} "    make  test I=spam O='-v -d 32'"
	@${ECHO} "    make vtest I=spam"

install:
	@${MKDIR} -p ${LOKISA_BIN}
	${CP} ${TARGETS} ${LOKISA_BIN}
	@${ECHO}
	@${ECHO} "make procmail to install ${HOME}/.procmailrc"

dbs: clean_dbs
	${LOKISA_BIN}/sa-dbm.pl -s ${S} -g ${G}
	${LOKISA_BIN}/sa-dbm.pl -w ${W}

clean_dbs:
	rm -f ${LOKISA}/*.pag
	rm -f ${LOKISA}/*.dir
	rm -f ${LOKISA}/*.db
	${LOKISA_BIN}/sa-dbm.pl -s /dev/null -g /dev/null -w /dev/null

spam_db:
	${LOKISA_BIN}/sa-dbm.pl -s ${S}

good_db:
	${LOKISA_BIN}/sa-dbm.pl -g ${G}

white_db:
	${LOKISA_BIN}/sa-dbm.pl -w ${W}

read_spam:
	${LOKISA_BIN}/sa-dbm.pl -r spam | sort

read_good:
	${LOKISA_BIN}/sa-dbm.pl -r good | sort

read_white:
	${LOKISA_BIN}/sa-dbm.pl -r white | sort

procmail:
	${CP} sa-procmail ${HOME}/.procmailrc

test:
	@(if [ -z "${I}" -o ! -r ${I} ]; then \
	${ECHO} "Define input, options via 'make ${.TARGET} I='...' [O='...']"; \
	else (time ./sa-filter.pl ${O} ${I}) | \
	    diff - ${I} || true; fi)

# This target is useful for "re-processing under other options for comparison"
# Note that we can't use sa-filter.pl -H for egrep, because that would filter
# out only the header of the current version.
vtest:
	@(if [ -z "${I}" -o ! -r ${I} ]; then \
	${ECHO} "Define input, options via 'make ${.TARGET} I='...' [O='...']"; \
	else (time ./sa-filter.pl ${O} ${I}) | \
	    egrep '^(From |Subject:|X-Lokisa:)' || true; fi)

