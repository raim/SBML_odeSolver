#!/usr/bin/awk -f
/\#t/
{
  printf("set data style linespoints\n");
  printf("plot");
  n = 0;
  for (i = 2; i != ARGC; i++)
  {
    for (j = 2; j != NF+1; j++)
    {
      if (ARGV[i] == $(j))
      {
        printf(" '");
	printf(ARGV[1]);
	printf("'");
	printf(" using 1:");
	printf(j);
	printf(" title '");
	printf($(j));
	printf("'");
	
	if (i != ARGC - 1)
	   printf(",");
	n++;
      }	      
    }      
  }
  exit 0;
}
