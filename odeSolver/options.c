/*
  Last changed Time-stamp: <2007-11-30 16:37:42 raim>
  $Id: options.c,v 1.9 2007/11/30 16:06:09 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>


/* own header files */
#include <sbmlsolver/util.h>

#include "options.h"

static struct option const long_options[] =
{ {"all",           no_argument,       0, 'a'},
  {"benchmark",     no_argument,       0, 'b'},
  {"compile",       no_argument,       0, 'c'},
  {"determinant",   no_argument,       0, 'd'},
  {"printmodel",    no_argument,       0, 'e'},
  {"onthefly",      no_argument,       0, 'f'},
  {"modelgraph",    no_argument,       0, 'g'},
  {"help",          no_argument,       0, 'h'},
  {"interactive",   no_argument,       0, 'i'},
  {"jacobian",      no_argument,       0, 'j'},
  {"reactions",     no_argument,       0, 'k'},
  {"message",       no_argument,       0, 'l'},
  {"matrixgraph",   no_argument,       0, 'm'},
  {"event",         no_argument,       0, 'n'},
  {"negative",      no_argument,       0, 'N'},
  {"printsbml",     no_argument,       0, 'o'},
  {"rates",         no_argument,       0, 'r'},
  {"steadyState",   no_argument,       0, 's'},
  {"sensitivity",   no_argument,       0, 't'},
  {"validate",      no_argument,       0, 'v'},
  {"write",         no_argument,       0, 'w'},
  {"xmgrace",       no_argument,       0, 'x'},
  {"jacobianTime",  no_argument,       0, 'y'},
  {"resetOnEvent",  no_argument,       0, 'z'},
  {"error",         required_argument, 0,   0},
  {"rerror",        required_argument, 0,   0},
  {"mxstep",        required_argument, 0,   0},
  {"gvformat",      required_argument, 0,   0},  
  {"threshold",     required_argument, 0,   0},
  {"printstep",     required_argument, 0,   0},
  {"method",        required_argument, 0,   0},
  {"iteration",     required_argument, 0,   0},
  {"model",         required_argument, 0,   0},
  {"mpath",         required_argument, 0,   0},
  {"param",         required_argument, 0,   0},
  {"schema11",      required_argument, 0,   0},
  {"schema12",      required_argument, 0,   0},
  {"schema21",      required_argument, 0,   0},
  {"spath",         required_argument, 0,   0},  
  {"time",          required_argument, 0,   0},
  
  {NULL, 0, NULL, 0}
};

/* forward declarations of (private) functions */
void initializeOptions (void);
static void processOptions (int argc, char *argv[]);

/**/
void 
decodeCML (int argc, char *argv[])
{
  initializeOptions();
  processOptions(argc, argv);
}

/**/
void
initializeOptions (void)
{
  strcpy(Opt.GvFormat,   "ps");
  /* was strcpy(Opt.ModelPath,  "./"); chnaged by AMF 9th March 2006 - seems unnecessary and
    blocks full path model reference on windows at least */
  strcpy(Opt.ModelPath,  "");
  strcpy(Opt.Parameter,  "");
  strcpy(Opt.SchemaPath, "./");
  strcpy(Opt.Schema11,   "sbml-l1v1.xsd");
  strcpy(Opt.Schema12,   "sbml-l1v2.xsd");
  strcpy(Opt.Schema21,   "sbml-l2v1.xsd");
  Opt.Error           = 1e-9;
  Opt.RError          = 1e-4;
  Opt.Mxstep          = 10000;
  Opt.ssThreshold     = 1e-11;
  Opt.Method          = 0;
  Opt.PrintStep       = 50;
  Opt.Time            = 1;
  Opt.HaltOnEvent     = 1;
  Opt.DetectNegState  = 0;
  Opt.InterActive     = 0;
  Opt.Jacobian        = 1;
  Opt.Determinant     = 0;
  Opt.DrawJacobian    = 0;
  Opt.PrintAll        = 0;
  Opt.PrintJacobian   = 0;
  Opt.PrintReactions  = 0;
  Opt.Xmgrace         = 0;
  Opt.DrawReactions   = 0;
  Opt.PrintModel      = 0;
  Opt.PrintODEsToSBML = 0;
  Opt.PrintOnTheFly   = 0;
  Opt.PrintMessage    = 0;
  Opt.Wheel           = 1;
  Opt.Sensitivity     = 0;
  Opt.SteadyState     = 0;
  Opt.Validate        = 0;
  Opt.Write           = 0;
  Opt.Compile         = 0;
  Opt.Benchmark       = 0;
  Opt.ResetCvodeOnEvents = 0;
}

/**/
static void
processOptions (int argc, char *argv[])
{
  int c, option_index = 0;

  /* memorize program name */  
  if (sscanf(argv[0], "%s", Opt.ExeName) == 0) {
    Warn (stderr, "%s:%d processOptions(): No ExeName found",
	  __FILE__, __LINE__);
    usage (EXIT_FAILURE);
  }
  
  /* process command line options */
  while ((c = getopt_long (argc, argv, "abcdefghijklmnorstvwxyzN",
                           long_options, &option_index)) != EOF) {
    switch (c) {
    case 0:
      if (strcmp(long_options[option_index].name, "gvformat")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No graph format specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { strcpy(Opt.GvFormat, tmp); }
      }
      if (strcmp(long_options[option_index].name, "param")==0) {
       char tmp[64];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No parameter specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { strcpy(Opt.Parameter, tmp); }
      }
      if (strcmp(long_options[option_index].name, "error")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr,
		"%s:%d processOptions(): No abs. Error tolerance specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.Error = tmp; }
      }
      if (strcmp(long_options[option_index].name, "rerror")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr,
		"%s:%d processOptions(): No rel. error tolerance specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.RError = tmp; }
      }
      if (strcmp(long_options[option_index].name, "printstep")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No PrintStep specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.PrintStep = tmp; }
      }
      if (strcmp(long_options[option_index].name, "method")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Mxstep specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.Method = tmp; }
      }
      if (strcmp(long_options[option_index].name, "iteration")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Mxstep specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.IterMethod = tmp; }
      }
      if (strcmp(long_options[option_index].name, "mxstep")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Mxstep specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.Mxstep = tmp; }
      }
      if (strcmp(long_options[option_index].name, "threshold")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr,
		"%s:%d processOptions(): No steady state threshold specified", 
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.ssThreshold = tmp; }
      }
      if (strcmp(long_options[option_index].name, "model")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No ModelFile specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.ModelFile, tmp); }
      }
      if (strcmp(long_options[option_index].name, "mpath")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No ModelPath specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.ModelPath, tmp); }
      }      
      if (strcmp(long_options[option_index].name, "schema11")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Schema L1v1 specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.Schema11, tmp); }
      }
      if (strcmp(long_options[option_index].name, "schema12")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Schema L1v2 specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.Schema12, tmp); }
      }
      if (strcmp(long_options[option_index].name, "schema21")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No Schema L2v1 specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.Schema21, tmp); }
      }
      if (strcmp(long_options[option_index].name, "spath")==0) {
        char tmp[256];
        if (sscanf(optarg, "%s", tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No SchemaPath specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        } else { strcpy(Opt.SchemaPath, tmp); }
      }
      if (strcmp(long_options[option_index].name, "time")==0) {
        double tmp;
        if (sscanf(optarg, "%lf", &tmp) == 0) {
          Warn (stderr, "%s:%d processOptions(): No time specified",
                __FILE__, __LINE__);
          usage (EXIT_FAILURE);
        }
        else { Opt.Time = tmp; }
      }
      break;
    case 'a':
      Opt.PrintAll = 1;
      break;
    case 'b':
      Opt.Benchmark = 1;
      break;
    case 'c':
      Opt.Compile = 1;
      break;
    case 'h':
      usage (EXIT_SUCCESS);
      break;
    case 'i':
      Opt.InterActive = 1;
      break;
    case 'j':
      Opt.Jacobian = 0;
      break;
    case 'y':
      Opt.PrintJacobian = 1;      
      break;
    case 'k':
      Opt.PrintReactions = 1;      
      break;
    case 'x':
      Opt.Xmgrace = 1;
      break;
    case 'g':
      Opt.DrawReactions = 1;
      break;
    case 'm':
      Opt.DrawJacobian = 1;
      break;
    case 'n':
      /* AMF removed 10 June 2005 Opt.PrintOnTheFly = 1; */
      Opt.HaltOnEvent   = 0;
      break;
    case 'N':
      Opt.DetectNegState = 1;
      break;
    case 'e':
      Opt.PrintModel = 1;
      break;
    case 'f':
      Opt.PrintOnTheFly = 1;
      break;
    case 'l':
      Opt.PrintMessage = 1;
      break;      
    case 'o':
      Opt.PrintODEsToSBML = 1;
      break;
    case 'd':
      Opt.Determinant = 1;
      break; 
    case 'r':
      Opt.PrintRates = 1;
      break;
    case 's':
      Opt.SteadyState = 1;
      break;
    case 't':
      Opt.Sensitivity = 1;
      break;
    case 'v':
      Opt.Validate = 1;
      break;	      
    case 'w':
      Opt.Write = 1;
      break;
    case 'z':
      Opt.ResetCvodeOnEvents = 1;
      break;
    default:
      usage (EXIT_FAILURE);
    }
  }
  /* parse the to file names from the remaining command-line */
  if ( strlen(Opt.ModelFile) > 0 ) return;
  else if ( (optind + 1) <= argc ) strcpy(Opt.ModelFile, argv[optind++]);
  else {
    Warn (stderr, "No ModelFile found on cmd-line");
    usage(EXIT_FAILURE);
  }
}

/**/
void
usage (int status)
{
  fprintf(stderr, "\nUSAGE:  %s <mpath/sbmlfile.xml> [OPTION]\n\n",
	  Opt.ExeName); 
  fprintf(stderr,
    "GENERAL OPTIONS\n"
    " -h, --help            Print (this) usage information.\n"
    " -i, --interactive     Turn on interactive mode\n"
    "     --gvformat <Str>  Set output format for graph drawings (now set \n"
    "                       to: %s); ignored if compiled w/o graphviz)\n",
	  Opt.GvFormat);
  fprintf(stderr,
   "SBML FILE PARSING\n"	  
    " -v, --validate        Validate SBML file before further processing\n"
	  
    "     --model <Str>     SBML file name (not needed!, see USAGE)\n"
    "                       (now set to: %s)\n"
    "     --mpath <Dir>     Set Model File Path\n"
	  "                       (now set to: %s)\n",
	  Opt.ModelFile, Opt.ModelPath);
  fprintf(stderr,
    "     --schema11 <Str>  Set filename for SBML schema Level 1 Version 1\n"
    "                       (now set to: %s)\n"
    "     --schema12 <Str>  Set filename for SBML schema Level 1 Version 2\n"
    "                       (now set to: %s)\n"
    "     --schema21 <Str>  Set filename for SBML schema Level 2 Version 1\n"
    "                       (now set to: %s)\n"
    "     --spath <Dir>     Set schema file path, absolute or relative to\n"
	  "                       model path (now set to: %s)\n",	  
	  Opt.Schema11, Opt.Schema12, Opt.Schema21,  Opt.SchemaPath);
  fprintf(stderr,
    "(1) PRINT REACTIONS AND DERIVED ODEs\n"
    " -e, --printmodel       Print Reactions and derived ODE system\n"
    " -o, --printsbml       Construct ODEs and print as SBML model\n"
    " -g, --modelgraph      Draw bipartite graph of reaction network\n"
    "                       (to .dot text file if compiled w/o graphviz)\n");
  fprintf(stderr,
    "(2) INTEGRATING\n"
    " -f, --onthefly        Print results during integration\n"
    " -l, --message         Print messages, and integration statistics\n"
    " -j, --jacobian        Toggle use of the jacobian matrix or CVODE's\n"
    "                       internal approximation (default: jacobian)\n");
  fprintf(stderr,
    " -s, --steadyState     Abort integration at steady states\n"
    " -t, --sensitivity     activate sensitivity analysis (default: no)\n"
    " -n, --event           Do not abort on event detection, but keep\n"
    "                       integrating. ACCURACY DEPENDS ON --printstep!!\n"
    " -c, --compile         Compile ODE, Jacobian and Event functions\n"
    " -b, --benchmark       Print execution duration and intergation duration\n"
    " -z, --resetOnEvent    Free and Restart CVODE when any event is triggered\n");
/*     "     --param <Str>     Choose a parameter to vary during batch\n" */
/*     "                       integration, from 0 to value in 50 steps\n"); */
  fprintf(stderr,
    "     --printstep <Int> Time steps of output, or\n"
    "                       (now set to: %g)\n"
    "     --time <Float>    Integration end time\n"
    "                       (now set to: %g)\n"	  
    "     --error <Float>   Absolute error tolerance during integration\n"
    "                       (now set to: %g)\n"
    "     --rerror <Float>  Relative error tolerance during integration\n"
    "                       (now set to: %g)\n"
    "     --threshold <Float> Threshold for steady state detection\n"
    "                       (now set to: %g\n"
    "     --mxstep <Int>    Maximum step number during integration\n"
    "                       (now set to: %g)\n",
	  Opt.PrintStep, Opt.Time, Opt.Error, Opt.RError, Opt.ssThreshold,
	  Opt.Mxstep);
  fprintf(stderr,
    "     --method <0/1>    Integration method, 0: BDF, 1: Adams-Moulton\n"
    "                       (now set to: %d)\n"
    "     --iteration <0/1> Iteration method, 0: Newton, 1: Functional\n"
    "                       (now set to: %d)\n",
	  Opt.Method, Opt.IterMethod); 
  fprintf(stderr,
    "(3) INTEGRATION RESULTS\n"
    " -a, --all             Print all available results (y/k/r + conc.).\n"
    " -y, --jacobianTime    Print time course of jacobian matrix entries,\n"
    "                       instead of concentrations\n"
    " -k, --reactions       Print time course of the reactions\n"
    "                       (kinetic laws) instead of concentrations\n"
    " -r, --rates           Print time course of the ODEs, instead of\n"
	  "                       concentrations\n");	  
  fprintf(stderr,
    " -w, --write           Write results to file (path/modelfile.xml.dat)\n"  
    " -x, --xmgrace         Print results to XMGrace; uses SBML Names\n"
    "                       instead of Ids (ignored if compiled w/o Grace)\n" 
    " -m, --matrixgraph     Draw species interactions from the jacobian\n"
    "                       matrix at last timepoint of integration\n"
    "                       (to .dot text file if compiled w/o graphviz)\n"
	  "\n");	  
  exit (status);
}

/* End of file */
