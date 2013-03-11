/* ---------------------------------------------------------------------- */
/* Test program for the dempster shafer rule                              */
/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dempster.h"

#define DEBUG_PRINT

#define FRAME_LOOK_BEHIND 7
basicMeasure * look_behind_buffer[FRAME_LOOK_BEHIND];


/*
eingabe:
    stirn -> (viel)angst | (wenig)wut
    unit5 -> angst/wut
    mund  -> (viel)freude | (wenig)angst
*/

float PIXEL_STIRN_MAX = 0;
float PIXEL_MUND_MAX  = 0;

//replace this sign with '.' for float numbers
#define KOMMA ','

//must be < 0.5
#define UNIT5_PROBABILITY 0.35
                          // A,W,F
const int ANGST_ENTRY[3]  = {1,0,0};
const int WUT_ENTRY[3]    = {0,1,0};
const int FREUDE_ENTRY[3] = {0,0,1};

char* GEFUEHLE_NAMEN[] = {"Angst  ","Wut    ","Freude "};


// *** print output ***
void print_plausibilities_cnt(int frame_nr,  int number_of_frames, basicMeasure *m)
{
    /* plausibilities */
    printf( "\nFrame %d bis %d\t Gefühl\t\t Nr : Pl(x)  |  B(x)   |  Z(x) \n" , frame_nr-number_of_frames>1?frame_nr-number_of_frames:1, frame_nr);
    int i;
    for (i=0;i<3;i++)
    {
        printf("\t\t(%s)\t[%d] : %5.3f  |  %5.3f  | %5.3f \n", GEFUEHLE_NAMEN[i], i, plausibility(m,i), singleBelief(m,i), singleDoubt(m,i));
    }
}

// *** print output ***
void print_plausibilities(int frame_nr, basicMeasure *m)
{
    /* plausibilities */
    printf( "\nFrame %d\t Gefühl\t\t Nr : Pl(x)  |  B(x)   |  Z(x) \n" , frame_nr);
    int i;
    for (i=0;i<3;i++)
    {
        printf("\t\t(%s)\t[%d] : %5.3f  |  %5.3f  | %5.3f \n", GEFUEHLE_NAMEN[i], i, plausibility(m,i), singleBelief(m,i), singleDoubt(m,i));
    }
}

// *** Read next line of file and retrive values for 'frame_nr', 'unit5', 'pixel of strin' and pixel of mund ***
int read_values_of_next_line(FILE *file, int *frame_nr, float *frame_unit5 , float *frame_stirn, float *frame_mund )
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if( (read = getline(&line, &len, file)) != -1)
    {
        #ifdef DEBUG_PRINT
        printf("Retrieved line of length %zu :\n", read);
        #endif
        char *p = strchr(line,KOMMA);
        while( p != NULL )
        {
            line[p-line] = '.';
            p = strchr(line,KOMMA);
        }
        #ifdef DEBUG_PRINT
        printf("> %s\n", line);
        read = sscanf( line , "%d;%f;%f;%f\n"  , frame_nr, frame_unit5 , frame_stirn, frame_mund );
        if(read > 0)
            printf( "READ >> FrameNr.:%d | Unit5: %.0f | Strin: %.2f | Mund: %.2f\n"  , *frame_nr, *frame_unit5 , *frame_stirn, *frame_mund );
        #endif
    }
    return read;
}

// *** Finde the maximum pixel for 'stirn' and 'mund' ***
void find_max_values(FILE *file)
{
    int frame_nr,read;
    float frame_unit5, frame_stirn,frame_mund;

    while( (read = read_values_of_next_line(file, &frame_nr, &frame_unit5 , &frame_stirn, &frame_mund )) != -1)
    {
        if(read > 0)
        {
            #ifdef DEBUG_PRINT
            printf("Found: Sitrn: %.2f | Mund: %.2f\n", frame_stirn, frame_mund);
            #endif
            if(PIXEL_STIRN_MAX < frame_stirn)
            {
                PIXEL_STIRN_MAX = frame_stirn;
                #ifdef DEBUG_PRINT
                printf("\tnew 'stirn' max\n");
                #endif
            }
            if(PIXEL_MUND_MAX < frame_mund)
            {
                PIXEL_MUND_MAX = frame_mund;
                #ifdef DEBUG_PRINT
                printf("\tnew 'mund' max\n");
                #endif
            }
        }
    }
    //there can always be an lager mouth so we add some pixel. and if we leave the maximum at the real maximum 
    // and wie devide the pixel count of a frame we get 1, and a propability of 1 is bad ;)
    PIXEL_STIRN_MAX += 50;
    PIXEL_MUND_MAX  += 30;
    printf("Max Values found: Stirn: %.2f  | Mund: %.2f \n", PIXEL_STIRN_MAX, PIXEL_MUND_MAX );
}



int parseEmotions(FILE *file, int *frame_nr, basicMeasure **res_ret)
{
    basicMeasure m_1,m_2,m_3,*res;

    set *ANGST,*WUT,*FREUDE;
    ANGST  = createAlternatives((int *)ANGST_ENTRY,3);
    WUT    = createAlternatives((int *)WUT_ENTRY,3);
    FREUDE = createAlternatives((int *)FREUDE_ENTRY,3);

    float frame_unit5, frame_stirn,frame_mund;

    //while( fscanf( file , "%d;%f;%f;%f\n"  , frame_nr, &frame_unit5 , &frame_stirn, &frame_mund ) <= 0 && !feof(file));
    int read;
    do
    {
        #ifdef DEBUG_PRINT
        printf("reading line...\n");
        #endif
        read = fscanf( file , "%d;%f;%f;%f\n"  , frame_nr, &frame_unit5 , &frame_stirn, &frame_mund );
        #ifdef DEBUG_PRINT
        printf("Read %d items\n",read);
        #endif
        if(read == 0 && !feof(file)) //skip missformed lines
        {
            char *c[50];
            fscanf( file , "%[^\t\n]",c);
            #ifdef DEBUG_PRINT
            printf("Missformed line: %s\n",c);  
            #endif
            continue;
        }
    }
    while(read <= 0 && !feof(file));
    if(feof(file))
    {
        #ifdef DEBUG_PRINT
        printf("eof reached\n");
        #endif
        return -1;
    }

    printf("Frame[%d] : Unit5: %.0f  | Stirn: %.0f  | Mund: %.0f \n", *frame_nr, frame_unit5, frame_stirn, frame_mund );

    //stirn
    createBasicMeasure(&m_1,3);
    addMeasureEntry(&m_1,*ANGST, frame_unit5*(frame_stirn/PIXEL_STIRN_MAX-0.1)+0.05 );
    addMeasureEntry(&m_1,*WUT, frame_unit5*(1-(frame_stirn/PIXEL_STIRN_MAX)-0.1)+0.05 );
    #ifdef DEBUG_PRINT
    printf("Stirn auswertung:");
    printBasicMeasure(&m_1);
    #endif

    //unit5
    createBasicMeasure(&m_2,3);
    addMeasureEntry(&m_2,*ANGST, frame_unit5*UNIT5_PROBABILITY+0.1 );
    addMeasureEntry(&m_2,*WUT, frame_unit5*UNIT5_PROBABILITY+0.1 );
    #ifdef DEBUG_PRINT
    printf("Unit5 auswertung:");
    printBasicMeasure(&m_2);
    #endif

    //mund
    createBasicMeasure(&m_3,3);
    addMeasureEntry(&m_3,*FREUDE, frame_mund/PIXEL_MUND_MAX-0.05 );
    addMeasureEntry(&m_3,*ANGST, 1-(frame_mund/PIXEL_MUND_MAX)-0.05 );
    #ifdef DEBUG_PRINT
    printf("Mund auswertung:");
    printBasicMeasure(&m_3);
    #endif

    #ifdef DEBUG_PRINT
    printf("accumulate ...\n");
    #endif
    res  = getAccumulatedMeasure(&m_1,&m_2);
    #ifdef DEBUG_PRINT
    printBasicMeasure(res);
    #endif
    *res_ret = getAccumulatedMeasure(&m_3,res);

    #ifdef DEBUG_PRINT
    printBasicMeasure(*res_ret);
    printf("line parsed\n");
    #endif

    return 1;
}

int parse_emotion_file(FILE *file, int *frame_nr, basicMeasure **ret_res)
{
    //create an empty basiModel for the return value, which holds all accumulated values
    createBasicMeasure(*ret_res,3);
    int i;
    while(!feof(file))
    {
        basicMeasure *bm;
        if(parseEmotions(file, frame_nr, &bm))
        {
            #ifdef DEBUG_PRINT
            printf("read line:");
            printBasicMeasure(bm);
            printf("read line output end:");
            #endif
            // plausibilities
            print_plausibilities(*frame_nr, bm);

            //for the overall accumulated measure
            *ret_res = getAccumulatedMeasure( *ret_res , bm);

            //Stuff for look-behind-buffer
            look_behind_buffer[(*frame_nr-1)%FRAME_LOOK_BEHIND] = bm;
            basicMeasure *tmp;
            for(i = 1, tmp = look_behind_buffer[0]; i < FRAME_LOOK_BEHIND && i < *frame_nr-1 ; i++)
            {
                tmp = getAccumulatedMeasure( tmp , look_behind_buffer[i]);
            }

            // plausibilities for last 7 Frames 
            printf("Accumulated measures over the last %d frames:", FRAME_LOOK_BEHIND);
            print_plausibilities_cnt(*frame_nr, FRAME_LOOK_BEHIND, tmp);
        }
        else
            return 1;
    }
}



int main(int argc, char *argv[])
{
    //open the csv file
    FILE *file;
    if(argc == 2)
        file = fopen ( argv[1], "r" );
    else
    {
        printf("Exactly one parameter expected. You must specify a file which should be read");
        return -1;
    }

    find_max_values(file);
    fseek(file, 0, SEEK_SET);

    //TODO alle frames zusammen accumulieren und am ende finales ergebniss ausgeben!
    basicMeasure *bm;
    int frame_count = 0;
    if(parse_emotion_file(file, &frame_count, &bm) > 0)
    {
        printf("read %d frames", frame_count);
        // overall plausibilities
        print_plausibilities_cnt(frame_count, frame_count, bm);

    }
    else
        printf("error while reading file");

    printf("\n\n");
    //close the csv file
    fclose ( file );
    return 0;
}


