/* ---------------------------------------------------------------------- */
/* Module  : evidences / dempster shafer                                  */
/* ---------------------------------------------------------------------- */
/* Filename        : dempster.h                                           */
/* Version History : 1.0 - 4.7.2004 uncommented running version           */
/*                   1.1 - 13.11.04 minor refinements                     */
/* Last Changes    : 13.11.04 Comments and refinements                    */
/* Author          : Dr. Dirk Reichardt / Berufsakademie Stuttgart        */
/* Created         : May 2004                                             */
/* ---------------------------------------------------------------------- */
/* Compiler Variables :   -                                               */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* This modules provides functions to handle evidences and accumulate     */
/* measures using the dempster shafer rule.                               */
/* ---------------------------------------------------------------------- */

#ifndef _DEMPSTER_SHAFER
#define _DEMPSTER_SHAFER

#include <stdio.h>


/* ---------------------------------------------------------------------- */

/* The maximum number of alternatives is set by a system constant (could  */
/* be replaced by dynamic memory allocation later).                       */

#define MAX_ALTERNATIVES 20


/* ---------------------------------------------------------------------- */
/* Types / Representation                                                 */
/* ---------------------------------------------------------------------- */


/* Set            ------------------------------------------------------- */

/* A simple set representation by marking an alternative as present (1)   */

typedef struct
{
  int alt[MAX_ALTERNATIVES];    /* array of values (0,1) [presence flags] */
  int size;                     /* number of fields used (leftbound)      */
} set;


/* basicMeasure   ------------------------------------------------------- */

/* A basic measure is represented by a linked list of subsets (pot) and   */
/* an associated evidence value for each subset.                          */

typedef struct m_m
{
  set   a;
  float value;          /* evidence value of set a (between 0 and 1)      */
  struct m_m * next;

} basicMeasure;


/* ---------------------------------------------------------------------- */
/* Function Prototypes                                                    */
/* ---------------------------------------------------------------------- */

/* Constructors --------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/* Function name    : createBasicMeasure                                  */
/* ---------------------------------------------------------------------- */
/* input parameters : basicMeasure *m  - pointer to an empty basic measure*/
/*                                       which is to be initialized       */
/*                    int size         - number of alternatives           */
/* output parameters: basicMeasure *m  - pointer to the filled structure  */
/* return value     : -                                                   */
/* used routines    : -                                                   */
/* error handling   : -                                                   */
/* ---------------------------------------------------------------------- */

void createBasicMeasure(basicMeasure *m, int size);

/* ---------------------------------------------------------------------- */
/* Function name    : createAlternatives                                  */
/* ---------------------------------------------------------------------- */
/* input parameters : int *a           - array of membership flags        */
/*                    int size         - number of alternatives           */
/* output parameters: -                                                   */
/* return value     : set *            - a filled (and allocated) set     */
/*                                       structure                        */
/* used routines    : -                                                   */
/* error handling   : corrects values of a beyond (0,1) and limits size   */
/*                    to the valid borders [0,MAX_ALTERNATIVES]           */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Transforms an array of membership flags to a set representation.       */
/* ---------------------------------------------------------------------- */

set * createAlternatives(int a[MAX_ALTERNATIVES], int size);


/* ---------------------------------------------------------------------- */
/* Function name    : addMeasureEntry                                     */
/* ---------------------------------------------------------------------- */
/* input parameters : basicMeasure *m  - existing measure (probably empty)*/
/*                    set p            - a new set of alternatives        */
/*                    float v          - evidence value for the set       */
/* output parameters: basicMeasure *m  - modified measure (p/v) added     */
/* return value     : -                                                   */
/* used routines    : emptyBasicMeasure, equalAlternatives                */
/* error handling   : -                                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Composing a basic measure by adding subsets and their evidences        */
/* ---------------------------------------------------------------------- */

void addMeasureEntry(basicMeasure *m, set p, float v);


/* ---------------------------------------------------------------------- */
/* Predicate Prototypes                                                   */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* Predicate name : completeAlternatives                                  */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Returns 1 if the parameter consists of a complete set of alternatives  */
/* (representing A), else 0.                                              */
/* ---------------------------------------------------------------------- */

int completeAlternatives(set *p);

/* ---------------------------------------------------------------------- */
/* Predicate name : emptyBasicMeasure                                     */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Returns 1 if the measure represents "no information" / "no evidence"   */
/* represented by a complete set of alternatives (A).                     */
/* ---------------------------------------------------------------------- */

int emptyBasicMeasure(basicMeasure *m);


/* Selectors ------------------------------------------------------------ */


/* ---------------------------------------------------------------------- */
/* Function name    : getBasicMeasure                                     */
/* ---------------------------------------------------------------------- */
/* input parameters : basicMeasure *m  - a basic measure in which to look */
/*                    set *p           - the alternative set to look up   */
/* output parameters: -                                                   */
/* return value     : float            - the evidence for the set p       */
/* used routines    : emptyBasicMeasure, completeAlternatives             */
/* error handling   : -                                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* the function looks up the evidence for a given alternative set p in a  */
/* basic measure m.                                                         */
/* ---------------------------------------------------------------------- */

float getBasicMeasure(basicMeasure *m, set *p);


/* ---------------------------------------------------------------------- */
/* Function name    : getAccumulatedMeasure                               */
/* ---------------------------------------------------------------------- */
/* input parameters : basicMeasure *m1 - a basic measure to accumulate    */
/*                    basicMeasure *m2 - a basic measure to accumulate    */
/* output parameters: -                                                   */
/* return value     : basicMeasure *   - the accumulated measure          */
/* used routines    : createBasicMeasure,addMeasureEntry,getIntersection  */
/* error handling   : -                                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* applies the dempster shafer rule to accumulate two measures.           */
/* ---------------------------------------------------------------------- */

basicMeasure * getAccumulatedMeasure (basicMeasure *m1, basicMeasure *m2);


/* ---------------------------------------------------------------------- */
/* Function name : plausibility                                           */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Determines the plausibility value for an alternative "index" for a     */
/* given basic measure m.                                                 */
/* ---------------------------------------------------------------------- */

float plausibility(basicMeasure *m, int index);

/* ---------------------------------------------------------------------- */
/* Function name : generateAlternatives                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* generates an alternative set by a binary coded number. for example:    */
/* index 6 results in an alternative set 0110.                            */
/* ---------------------------------------------------------------------- */

set generateAlternatives(int size, int index);


/* Auxiliaries ---------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* Function name : singleBelief                                           */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* computes the belief in a specific alternative (determined by "index")  */
/* ---------------------------------------------------------------------- */

float singleBelief (basicMeasure *m, int index);

/* ---------------------------------------------------------------------- */
/* Function name : singleDoubt                                            */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* computes the doubt in a specific alternative (determined by "index")   */
/* ---------------------------------------------------------------------- */

float singleDoubt (basicMeasure *m, int index);


/* Destructor  ---------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* Function name : deleteBasicMeasure                                     */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* deallocates the dynamically allocated memory for the basic measure m.  */
/* ---------------------------------------------------------------------- */

void deleteBasicMeasure(basicMeasure *m);



/* ---------------------------------------------------------------------- */
/* I/O Auxiliaries                                                        */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/* Function name : printBasicMeasure                                      */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Output of a printed version of the measurement (screen)                */
/* ---------------------------------------------------------------------- */

void printBasicMeasure(basicMeasure *m);

#endif
