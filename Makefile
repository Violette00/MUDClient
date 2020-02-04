TARGETS = all test clean
PROJECTS = mudclient telnet common

${TARGETS}: ${PROJECTS}

${PROJECTS}:
	$(MAKE) -C $@ ${MAKECMDGOALS}

.PHONY: ${TARGETS} ${PROJECTS}
