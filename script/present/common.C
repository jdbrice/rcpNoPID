#ifndef PRESENT_COMMON
#define PRESENT_COMMON

vector<int> colors 		= { kRed + 1, 	kAzure - 3, 	kGreen + 1,		 kMagenta + 3, 		kOrange - 3, 	kYellow + 1, kCyan	};

vector<string> centralities = { "0", "1", "2", "3", "4", "5", "6" };
vector<string> centrality_labels = { "0-5%", "5-10%", "10-20%", "20-30%", "30-40%", "40-60%", "60-80%" };

vector<string> rcentralities = { "0", "1", "2", "3+4", "5", "6" };
vector<string> rcentrality_labels = { "0-5%", "5-10%", "10-20%", "20-40%", "40-60%", "60-80%" };

vector<string> energies =  { "7.7", 		"11.5", 	"14.5", 	"19.6", 		"27.0", 		"39.0" };
vector<string> renergies =  { "39.0",         "27.0",     "19.6",    "14.5",         "11.5",         "7.7" };

vector<int> rcpcolors      = { kAzure + 3, kAzure - 3,     kGreen + 1,      kMagenta + 3,      kOrange + 7,     kBlue + 3, kRed + 1  };
vector<string> rcpenergies =  { "62.4",     "39.0",         "27.0",     "19.6",         "11.5",         "7.7" };
vector<string> nullenergies =  { "", "",            "",         "",             "",             ""};


map<string, vector<double> > n_coll = 
{
	{"7.7", 	{ 773.78160, 628.57471, 450.06667, 283.09279, 170.62760, 74.26815, 19.19978} },
    {"11.5", 	{  784.45235, 635.10335, 453.30864, 283.86052, 171.77151, 74.74965, 19.08872} },
    {"14.5", 	{  787.9, 639, 455, 284, 171, 75, 18.305 } },
    {"19.6", 	{  799.84, 642.84, 458.4, 284.54252, 169.88653, 74.01196, 18.91901 } },
    {"27.0", 	{  841.47851, 693.95012, 496.90694, 311.59162, 187.65520, 81.95383, 19.97044 } },
	{"39.0", 	{  852.69718, 687.23303, 491.68086, 305.79089, 182.57132, 78.54930, 19.39689 } },
    {"62.4",    {  903.77238, 726.72764, 518.58367, 321.91242, 192.45834, 82.38742, 19.33833 }}
};

map<string, vector<double> > n_coll_unc = 
{
	{"7.7", 	{  28.16378, 0, 0, 0, 0, 16.42753, 6.25394 } },
    {"11.5", 	{  27.22447, 0, 0, 0, 0, 15.85789, 7.76079} },
    {"14.5", 	{  29.89010, 0, 0, 0, 0, 15.1698, 6.34844 } },
    {"19.6", 	{  27.37286, 0, 0, 0, 0, 15.39278, 6.86227 } },
    {"27.0", 	{  28.39811, 0, 0, 0, 0, 18.40486, 8.56423} },
	{"39.0", 	{  26.86605, 0, 0, 0, 0, 17.13290, 7.73350 } },
    {"62.4",    {  26.97008, 0, 0, 0, 0, 17.90687, 7.71289}}
};


#endif