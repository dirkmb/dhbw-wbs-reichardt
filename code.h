//#define DEBUG_PRINT

int FRAME_LOOK_BEHIND=7;
//basicMeasure * look_behind_buffer[FRAME_LOOK_BEHIND];
basicMeasure * look_behind_buffer[];


/*
eingabe:
    stirn -> (viel)angst | (wenig)wut
    unit5 -> angst/wut
    mund  -> (viel)freude | (wenig)angst
*/

float PIXEL_STIRN_MAX = 0;
float PIXEL_MUND_MAX  = 0;
float PIXEL_STIRN_MIN = 0; 
float PIXEL_MUND_MIN  = 0;

#define UNIT5_PROBABILITY 0.7

//wie warscheinlich ist es das keiner unserer ausgelesenen werte passt.
#define REMAINING_PROBABILITY 0.3


//replace this sign with '.' for float numbers
#define KOMMA ','

                             // A,W,F
const int ANGST_ENTRY[3]     = {1,0,0};
const int WUT_ENTRY[3]       = {0,1,0};
const int WUT_ANGST_ENTRY[3] = {1,1,0};
const int FREUDE_ENTRY[3]    = {0,0,1};

char* GEFUEHLE_NAMEN[] = {"Angst  ","Wut    ","Freude "};

// the corresbonding bits will be set if we recognize a feeling
int RESULT_ARRAY[3]    = {0,0,0};
float RECOGNIZE_PROBABILITY_THRESHOLD = 0.9;



