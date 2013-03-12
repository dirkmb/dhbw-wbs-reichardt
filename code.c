/* ---------------------------------------------------------------------- */
/* WBS dempster programm                                                  */
/* Author: Alexander Penack, Dirk Braunschweiger                          */
/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dempster.h"
#include "code.h"

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
    printf( "\nFrame %3d\t Gefühl\t\t Nr : Pl(x)  |  B(x)   |  Z(x) \n" , frame_nr);
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
    while( (read = getline(&line, &len, file)) != -1 )
    {
        #ifdef DEBUG_PRINT
        printf("Retrieved line of length %zu :\n", read);
        #endif

        //replace all KOMMAs with DOTs, so we can use scanf for reading floats
        char *p = strchr(line,KOMMA);
        while( p != NULL )
        {
            line[p-line] = '.';
            p = strchr(line,KOMMA);
        }
        //read the data
        read = sscanf( line , "%d;%f;%f;%f\n"  , frame_nr, frame_unit5 , frame_stirn, frame_mund );

        #ifdef DEBUG_PRINT
        printf("> %s", line);
        if(read > 0)
            printf( "READ >> FrameNr.:%d | Unit5: %.0f | Strin: %.2f | Mund: %.2f\n"  , *frame_nr, *frame_unit5 , *frame_stirn, *frame_mund );
        else
            printf( "skiping missformed line...\n\n");
        #endif

        if(read > 0)
            return read;
    }
    return read;
}

// *** Find the minimum and maximum pixel for 'stirn' and 'mund' ***
void find_min_max_values(FILE *file)
{
    int frame_nr,read;
    float frame_unit5, frame_stirn,frame_mund;

    while( (read = read_values_of_next_line(file, &frame_nr, &frame_unit5 , &frame_stirn, &frame_mund )) != -1 )
    {
        if(read > 0)
        {
            #ifdef DEBUG_PRINT
            printf("Found: Sitrn: %.2f | Mund: %.2f\n", frame_stirn, frame_mund);
            #endif

            // *** min values ***
            if(PIXEL_STIRN_MIN > frame_stirn || PIXEL_STIRN_MIN == 0)
            {
                PIXEL_STIRN_MIN = frame_stirn;
                #ifdef DEBUG_PRINT
                printf("\tnew 'stirn' min\n");
                #endif
            }
            if(PIXEL_MUND_MIN > frame_mund || PIXEL_MUND_MIN == 0)
            {
                PIXEL_MUND_MIN = frame_mund;
                #ifdef DEBUG_PRINT
                printf("\tnew 'mund' min\n");
                #endif
            }

            // *** max values ***
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
    // same here but make sure that the values do not get smaller than 0
    PIXEL_STIRN_MIN -= PIXEL_STIRN_MIN>50?50:PIXEL_STIRN_MIN/2;
    PIXEL_MUND_MIN  -= PIXEL_MUND_MIN>30?30:PIXEL_MUND_MIN/2;

    printf("Min Values: Stirn: %.2f \t| Mund: %.2f \n", PIXEL_STIRN_MIN, PIXEL_MUND_MIN );
    printf("Max Values: Stirn: %.2f \t| Mund: %.2f \n", PIXEL_STIRN_MAX, PIXEL_MUND_MAX );
}



// *** read next line of file and create calcultae probabilities, the basicMeasure and frame_nr 
///         of the current frame is 'returned' in frame_nr/res_ret parameter ***
int parseEmotions(FILE *file, int *frame_nr, basicMeasure **res_ret)
{
    basicMeasure m_1,m_2,m_3,*res;

    set *ANGST,*WUT,*WUT_ANGST,*FREUDE;
    ANGST     = createAlternatives((int *)ANGST_ENTRY,3);
    WUT       = createAlternatives((int *)WUT_ENTRY,3);
    WUT_ANGST = createAlternatives((int *)WUT_ANGST_ENTRY,3);
    FREUDE    = createAlternatives((int *)FREUDE_ENTRY,3);

    float frame_unit5, frame_stirn,frame_mund;

    int read = read_values_of_next_line(file, frame_nr, &frame_unit5 , &frame_stirn, &frame_mund );
    if ( read == -1 || read == 0 )
    {
        #ifdef DEBUG_PRINT
        printf("could not read another line");
        #endif
        return -1;
    }

    printf("\n- - - - -\n\nread next frame:  FrameNr.:%3d | Unit5: %.0f | Stirn: %.0f | Mund: %.0f \n", *frame_nr, frame_unit5, frame_stirn, frame_mund );

    // ** stirn **
    createBasicMeasure(&m_1,3);
    float a1 = (frame_stirn-PIXEL_STIRN_MIN)/(PIXEL_STIRN_MAX-PIXEL_STIRN_MIN);
    float w1 = 1-a1;
    a1 -= a1>REMAINING_PROBABILITY/2 ? REMAINING_PROBABILITY/2 : 0;
    w1 -= w1>REMAINING_PROBABILITY/2 ? REMAINING_PROBABILITY/2 : 0;

    addMeasureEntry(&m_1,*ANGST, a1<0?0:a1 );
    addMeasureEntry(&m_1,*WUT  , w1<0?0:w1 );
    #ifdef DEBUG_PRINT
    printf("Stirn auswertung:");
    printBasicMeasure(&m_1);
    #endif


    // ** unit5 **
    createBasicMeasure(&m_2,3);
    float wa = frame_unit5*UNIT5_PROBABILITY;
    wa += frame_unit5==0?REMAINING_PROBABILITY:-REMAINING_PROBABILITY;
    addMeasureEntry(&m_2,*WUT_ANGST, wa );
    #ifdef DEBUG_PRINT
    printf("Unit5 auswertung:");
    printBasicMeasure(&m_2);
    #endif


    // ** mund **
    createBasicMeasure(&m_3,3);
    float f3 = (frame_mund-PIXEL_MUND_MIN)/(PIXEL_MUND_MAX-PIXEL_MUND_MIN);
    float a3 = 1-f3;
    f3 -= f3>REMAINING_PROBABILITY/2 ? REMAINING_PROBABILITY/2 : 0;
    a3 -= a3>REMAINING_PROBABILITY/2 ? REMAINING_PROBABILITY/2 : 0;

    addMeasureEntry(&m_3,*FREUDE, f3<0?0:f3 );
    addMeasureEntry(&m_3,*ANGST , a3<0?0:a3 );
    #ifdef DEBUG_PRINT
    printf("Mund auswertung:");
    printBasicMeasure(&m_3);
    #endif


    // *** zusammenfassen ***
    #ifdef DEBUG_PRINT
    printf("accumulate ...\n");
    #endif
    res  = getAccumulatedMeasure(&m_1,&m_2);
    *res_ret = getAccumulatedMeasure(&m_3,res);

    #ifdef DEBUG_PRINT
    printBasicMeasure(res);
    printBasicMeasure(*res_ret);
    printf("line parsed\n");
    #endif

    return 1;
}


// *** calls 'parseEmotions' for parsing the next line of the file and calculats the overall measure and the 'look-behind- measure and print the results ***
int parse_emotion_file(FILE *file, int *frame_nr, basicMeasure **ret_res)
{
    //create an empty basiModel for the return value, which holds all accumulated values
    //createBasicMeasure(ret_res,3);

    int i;
    while(!feof(file))
    {
        basicMeasure *bm;
        if(parseEmotions(file, frame_nr, &bm) > 0)
        {
            #ifdef DEBUG_PRINT
            printf("read line:");
            printBasicMeasure(bm);
            printf("read line output end:");
            #endif
            // print plausibilities
            print_plausibilities(*frame_nr, bm);

            // ** calculate the overall accumulated measure **
            if(*ret_res == NULL)
                *ret_res = bm;
            else
                *ret_res = getAccumulatedMeasure( *ret_res , bm);

            // ** calculate the 'look-behind-buffer' measure **
            look_behind_buffer[(*frame_nr-1)%FRAME_LOOK_BEHIND] = bm;
            basicMeasure *tmp;
            for(i = 1, tmp = look_behind_buffer[0]; i < FRAME_LOOK_BEHIND && i < *frame_nr-1 ; i++)
                tmp = getAccumulatedMeasure( tmp , look_behind_buffer[i]);

            // print plausibilities for last 7 Frames  (look-behind-buffer-measure)
            printf("\nAccumulated measures over the last %d frames:", FRAME_LOOK_BEHIND);
            print_plausibilities_cnt(*frame_nr, FRAME_LOOK_BEHIND, tmp);

            // check it there is a recognizable feeling
            if(*frame_nr >= FRAME_LOOK_BEHIND)
            {
                for (i=0;i<3;i++)
                {
    //                printf("\t\t(%s)\t[%d] : %5.3f  |  %5.3f  | %5.3f \n", GEFUEHLE_NAMEN[i], i, plausibility(m,i), singleBelief(m,i), singleDoubt(m,i));
                    if( plausibility(tmp,i) >= RECOGNIZE_PROBABILITY_THRESHOLD )
                    {
                        RESULT_ARRAY[i] = 1;
                        printf("Feeling found: '%s'[%d]\n" , GEFUEHLE_NAMEN[i], i);
                    }
                }
            }
        }
        else
            return 1;
    }
    return -1;
}



int main(int argc, char *argv[])
{
    //open the csv file
    FILE *file;
    int i;
    if(argc >= 2  && argc <= 4)
    {
        file = fopen ( argv[1], "r" );
        if(argc >= 3) //probability
        {
            sscanf(argv[2] , "%f", &RECOGNIZE_PROBABILITY_THRESHOLD);
        }
        if(argc >= 4) //lookback
        {
            //read max look behind
            sscanf(argv[3] , "%d", &FRAME_LOOK_BEHIND);
            if(FRAME_LOOK_BEHIND <= 0)
                FRAME_LOOK_BEHIND = 1;

            //create a new look_behind_buffer
            basicMeasure *lbb[FRAME_LOOK_BEHIND];
            *look_behind_buffer = lbb;
        }
    }
    else
    {
        printf("Usage: dhbw-wbs-reichardt file [probability] [look-back]\n");
        printf("Parameters:\n");
        printf("<filename>\t\tName of a file with ';'-seperated values which should be parsed\n");
        printf("<propability>\t\tat which threshold a feeling should be recognized\n");
        printf("<lock-back>\t\thow many frames should be saved in the look-back buffer\n");
        return -1;
    }


    find_min_max_values(file);
    fseek(file, 0, SEEK_SET);

    basicMeasure *bm = NULL;
    int frame_count = 0;
    if(parse_emotion_file(file, &frame_count, &bm) > 0)
    {
        printf("\n----- ----- ----- -----\n\nreading and calculating completetd\nread %d frames\nOverall plausibility (note: zero if there are more than one feeling in the file)", frame_count);
        // overall plausibilities
        print_plausibilities_cnt(frame_count, frame_count, bm);

        printf("\n\nFollowing feelings have been recognized with mind %.3f:\n" , RECOGNIZE_PROBABILITY_THRESHOLD);
        for(i = 0 ; i < 3 ; i++)
            if(RESULT_ARRAY[i] == 1)
                printf("\t\t%s\n" , GEFUEHLE_NAMEN[i]);

    }
    else
        printf("error while reading file");

    printf("\n\n");

    //close the csv file
    fclose ( file );
    return 0;
}


