#include "bot.h"

int main(int argc, char** argv)
{
	ConnectionDispatcher d;

#include "botconfig.h"

	Bot bot(config);
	bot.connect(&d);
	
	while(true)
	{
		d.handle();
	}

	return 0;
}
