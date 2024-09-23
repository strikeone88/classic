/*
    RULE.H

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __RULE_H
#define __RULE_H

    #ifndef __BASE_H
    #include <base.h>
    #endif

    /* Pushes an empty rule into the rule stack. */
    #define pushEmptyRule() list__add (rule__stack, new (rule_t))

    /* Pops a rule from the rule stack. */
    #define popRule() (rule_t *)(list__pop (rule__stack))

    /* Rule structure/ */
    typedef struct rule_s
    {
        linkable_t  link;

        uint_t      count, line;
        int         form;

        struct      rule_s **rule;
        char        **value;
        uchar_t     *index;

        unsigned    long LocationCounter;
    }
    rule_t;

    /* Rule stack. */
    extern list_t *rule__stack;

    /* Initializes the module. */
    void iRule (void);

    /* Deinitializes the module. */
    void diRule (void);

    /* Destroys a rule (releases its memory entirely). */
    void dRule (rule_t *r);

    /* Iterates through a rule having a form of a list. */
    void iterateList (rule_t *r, void *func, void *parm1, void *parm2);

    /* Converts the rule into an string. */
    void RuleToString (rule_t *r, char *dest);

#endif
