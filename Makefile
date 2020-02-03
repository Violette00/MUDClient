TARGETS = all test clean
#PROJECTS = common mudclient telnet
PROJECTS = mudclient telnet

${TARGETS}: ${PROJECTS}

${PROJECTS}:
	$(MAKE) -C $@ ${MAKECMDGOALS}

.PHONY: ${TARGETS} ${PROJECTS}
