TARGETS = all test clean
#PROJECTS = common mudclient telnet
PROJECTS = 

${TARGETS}: ${PROJECTS}

${PROJECTS}:
	$(MAKE) -C $@ ${MAKECMDGOALS}

.PHONY: ${TARGETS} ${PROJECTS}
