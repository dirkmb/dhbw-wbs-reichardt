/* ---------------------------------------------------------------------- */
/* Module  : evidences / dempster shafer                                  */
/* ---------------------------------------------------------------------- */
/* Filename        : dempster.c                                           */
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

#include <math.h>
#include <stdlib.h>

#include "dempster.h"

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

void createBasicMeasure(basicMeasure *m, int size)
{
  int i;

  m->a.size  = size;         /* setting the alternatives in the structure */
  m->value   = 1.0f;         /* default evidence for the alternative set  */

  /* all alternatives are marked by 1 - signifying the complete set of    */
  /* alternatives (A).                                                    */

  for (i=0;i<m->a.size;i++)
  {
    m->a.alt[i] = 1;
  }

  /* no further elements belong to the core of the measurement            */

  m->next = NULL;
}


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

set * createAlternatives(int a[MAX_ALTERNATIVES], int size)
{
  set *ret;
  int i;

  ret = (set *)malloc(sizeof(set));

  ret->size = size;

  /* checking for invalid values of size parameter                        */

  if (size > MAX_ALTERNATIVES)
    ret->size = MAX_ALTERNATIVES;
  if (size < 0)
    ret->size = 0;

  for(i=0;i<ret->size;i++)
  {
    /* correcting invalid flags: if >= 1 assume 1 else assume 0           */

    if (a[i] >= 1)
      ret->alt[i] = 1;
    else
      ret->alt[i] = 0;
  }

  return ret;
}


int completeAlternatives(set *p)
{
  int ret = 0;
  int i;

  for(i=0;i<p->size;i++)
  {
    ret += p->alt[i];
  }

  return (ret == p->size);
}

int emptyBasicMeasure(basicMeasure *m)
{
  return (completeAlternatives(&(m->a)));
}

int equalAlternatives(set *m1, set *m2)
{
  int i;
  int ret = 1;
  for (i=0;i<m1->size;i++)
  {
    ret = ret * (m1->alt[i] == m2->alt[i]);
  }

  return ret;
}

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
/* basic measure.                                                         */
/* ---------------------------------------------------------------------- */

float getBasicMeasure(basicMeasure *m, set *p)
{
  basicMeasure *tmp;
  float ret;

  if (emptyBasicMeasure(m))
  {
    if (completeAlternatives(p))
    {
      ret = m->value;
    }
    else
      ret = 0.0f;
  }
  else
  {
    if (equalAlternatives(&(m->a),p))
    {
      ret = m->value;
    }
    else
    {
      ret = getBasicMeasure(m->next,p);
    }
  }
  return ret;
}

/* ---------------------------------------------------------------------- */
/* Function name    : addMeasureEntry                                     */
/* ---------------------------------------------------------------------- */
/* input parameters : basicMeasure *m  - a basic measure to extend        */
/*                    set *p           - the alternative set enter        */
/*                    float v          - the evidence for this set        */
/* output parameters: basicMeasure *m  - the extended basic measure       */
/* return value     : -                                                   */
/* used routines    : emptyBasicMeasure, equalAlternatives                */
/* error handling   : -                                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* an evidence value is assigned to a subset of alternatives within a     */
/* given measure.                                                         */
/* ---------------------------------------------------------------------- */

void addMeasureEntry(basicMeasure *m, set p, float v)
{
  basicMeasure *tmp;
  int i;

  tmp = m;

  /* go through until alternative set is found or it is not included */

  while((!(emptyBasicMeasure(tmp))) && (!(equalAlternatives(&(tmp->a),&p))))
  {
    tmp = tmp->next;
  }

  if (equalAlternatives(&(tmp->a),&p))
  {
    /* it was already included */
    tmp->value += v;
  }
  else if (emptyBasicMeasure(tmp))
  {
    /* it is not yet included ...*/
    tmp = m->next;

    /* allocate memory for the new entry */
    m->next = (basicMeasure *)malloc(sizeof(basicMeasure));

    /* move the first entry to the new element */
    m->next->value = m->value;
    m->next->next  = tmp;
    m->next->a     = m->a;

    /* move the new entry to the first field */
    m->value = v;
    m->a     = p;
  }

  /* update assignment to "A" */

  tmp = m;

  while (!(emptyBasicMeasure(tmp)))
  {
    tmp = tmp->next;
  }

  tmp->value -= v;
}


int emptySet(set p)
{
  int i;
  int ret = 0;
  for(i=0;i<p.size;i++)
  {
    ret+= p.alt[i];
  }

  return (ret == 0);
}

set getIntersection(set *p1, set *p2)
{
  int i;
  set ret;

  ret.size = p1->size;

  for(i=0;i<ret.size;i++)
  {
    ret.alt[i] = (p1->alt[i] && p2->alt[i]);
  }

  return ret;
}

float getConflict(basicMeasure *m1, basicMeasure *m2)
{
  float conflict = 0.0f;
  basicMeasure *tmp1,*tmp2;

  tmp1 = m1;

  while(!emptyBasicMeasure(tmp1))
  {
    tmp2 = m2;
    while(!(emptyBasicMeasure(tmp2)))
    {
      if (emptySet(getIntersection(&(tmp1->a),&(tmp2->a))))
      {
        conflict += tmp1->value * tmp2->value;
      }
      tmp2 = tmp2->next;
    }
    tmp1=tmp1->next;
  }

  return conflict;
}

basicMeasure * getAccumulatedMeasure (basicMeasure *m1, basicMeasure *m2)
{
  basicMeasure *ret;
  basicMeasure *tmp1,*tmp2;
  set          p;
  float val;
  float conflict;
  float correction;

  ret = (basicMeasure *)malloc(sizeof(basicMeasure));

  conflict = getConflict(m1,m2);

  if (conflict <= 0.99f)
  {
    correction = 1.0f/(1.0f-conflict);

    createBasicMeasure(ret,m1->a.size);

    tmp1 = m1;
    while(!(tmp1 == NULL))
    {
      tmp2 = m2;
      while (!(tmp2 == NULL))
      {
        p = getIntersection(&(tmp1->a),&(tmp2->a));

        val = tmp1->value * tmp2->value * correction;

        if ((!emptySet(p)) && (val > 0.0f))
          addMeasureEntry(ret,p,val);

        tmp2=tmp2->next;
      }
      tmp1=tmp1->next;
    }
  }

  return ret;
}

/* ---------------------------------------------------------------------- */
/* Function name : plausibility                                           */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* Determines the plausibility value for an alternative "index" for a     */
/* given basic measure m.                                                 */
/* ---------------------------------------------------------------------- */

float plausibility(basicMeasure *m, int index)
{
  float p = 0.0f;
  basicMeasure *tmp;

  tmp = m;

  if (!((index < 0) || (index > m->a.size -1)))
  {
    while (!(tmp == NULL))
    {
      if (tmp->a.alt[index] == 1)
      {
        p += tmp->value;
      }

      tmp = tmp->next;
    }
  }

  return p;
}

/* ---------------------------------------------------------------------- */
/* Function name : generateAlternatives                                   */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* generates an alternative set by a binary coded number. for example:    */
/* index 6 results in an alternative set 0110.                            */
/* ---------------------------------------------------------------------- */

set generateAlternatives(int size, int index)
{
  set p;
  int i,ind;
  int base = 1;

  p.size = size;

  base = (int)pow(2,size-1);

  ind = index;
  for(i=0;i<size;i++)
  {
     p.alt[i] = (ind / base);
     ind = ind % base;
     base = base/2;
  }
  return p;
}

int isSubset(set *p, set *q)
{
  int i;
  int ret = 1;

  for(i=0;i<p->size;i++)
  {
    if ((q->alt[i] == 0) &&
        (p->alt[i] == 1))
     ret = 0;
  }
  return ret;
}

set invert(set *p)
{
  set ret;
  int i;

  ret.size = p->size;
  for(i=0;i<p->size;i++)
    ret.alt[i] = (p->alt[i] == 0);

  return ret;
}

float belief(basicMeasure *m, set *p)
{
  int i,lim;
  set pt;
  float res = 0.0f;

  pt.size = p->size;

  lim = (int)pow(2,p->size);

  for (i=0;i<lim;i++)
  {
    pt = generateAlternatives(p->size,i);
    if (isSubset(&pt,p))
    {
      res += getBasicMeasure(m,&pt);
    }
  }

  return res;
}

float singleBelief (basicMeasure *m, int index)
{
  set p;
  int i;

  for (i=0;i<m->a.size;i++)
  {
    p.alt[i] = 0;
  }
  p.alt[index] = 1;

  p.size = m->a.size;

  return belief(m,&p);
}


float singleDoubt (basicMeasure *m, int index)
{
  set p;
  int i;

  for (i=0;i<m->a.size;i++)
  {
    p.alt[i] = 1;
  }
  p.alt[index] = 0;

  p.size = m->a.size;

  return belief(m,&p);
}

/* ---------------------------------------------------------------------- */
/* Function name : deleteBasicMeasure                                     */
/* ---------------------------------------------------------------------- */
/* Description :                                                          */
/* deallocates the dynamically allocated memory for the basic measure m.  */
/* ---------------------------------------------------------------------- */

void deleteBasicMeasure(basicMeasure *m)
{
  if (m->next == NULL)
  {
    free(m);
  }
  else
  {
    deleteBasicMeasure(m->next);
    free(m);
  }
}


void printBasicMeasure(basicMeasure *m)
{
  basicMeasure *tmp;
  int i;

  printf("Basic Measure:\n");
  tmp = m;
  while(tmp != NULL)
  {
    printf("m([");
    for(i=0;i<tmp->a.size;i++)
      printf("%1d",tmp->a.alt[i]);
    printf("]) = %7.3f\n",tmp->value);
    tmp = tmp->next;
  }
}

