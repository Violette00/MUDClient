typedef enum {
	SE=240,		/* Subnegotiation end */
	NOP=241,	/* No op */
	DM=242,		/* Data Mark */
	BRK=243,	/* Break */
	IP=244,		/* Interrupt Process */
	AO=245,		/* Abort output */
	AYT=246,	/* Are you there? */
	EC=247,		/* Erase Character */
	EL=248,		/* Erase Line */
	GA=249,		/* Go Ahead */
	SB=250,		/* Subnegotiation begin */
	WILL=251,	/* Will option */
	WONT=252,	/* Wont option */
	DO=253,		/* Do option */
	DONT=254,	/* Dont option */
	IAC=255,	/* Interpret as Command */
} TelnetCommand;

typedef struct {
	unsigned char command;
	unsigned char option[16];	/* used for will, wont, do, dont */
	unsigned int opt_count;
} Command;

int process_commands(const unsigned char *in, unsigned char *out,
		Command *commands);
