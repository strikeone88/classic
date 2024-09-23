/*
    RULE.C

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <sstring.h>
    #include <rule.h>
    #include <pcc.h>

    /* Stack of rules. */
    list_t *rule__stack;

    /* Initializes the module. */
    void iRule (void)
    {
            rule__stack = new (list_t);
    }

    /* Deinitializes the module. */
    void diRule (void)
    {
        rule_t *r;

            while ((r = popRule ()) != NULL)
                dRule (r);
    }

    /* Destroys a rule (releases its memory entirely). */
    void dRule (rule_t *r)
    {
        int i;

            if (!r) return;

            for (i = 0; i < r->count; i++)
            {
                if (r->value [i])
                    dString (r->value [i]);
                else
                    dRule (r->rule [i]);
            }

            delete (r->value);
            delete (r->index);
            delete (r->rule);

            delete (r);
    }

    /* Returns the logical location counter of the current section. */
    unsigned long locationCounter (void);

    /* Generically pushes a rule on the rule stack. */
    rACTION (pushRule)
    {
        rule_t *rule = new (rule_t);
        int i;

            rule->value = alloc (n * sizeof (char *));
            rule->rule = alloc (n * sizeof (rule_t *));
            rule->line = linenum;

            rule->index = alloc (n);

            rule->form = form;
            rule->count = n;

            rule->LocationCounter = locationCounter ();

            for (i = n - 1; i >= 0; i--)
            {
                rule->index [i] = ndx [i];

                if (argv [i][0] == '\0')
                    rule->rule [i] = list__pop (rule__stack);
                else
                    rule->value [i] = aString (argv [i]);
            }

            list__add (rule__stack, rule);
    }

    /* Pushes an empty rule. */
    rACTION (pushEmpty)
    {
            pushEmptyRule ();
    }

    /* Iterates through a rule having a form of a list. */
    void iterateList (rule_t *r, void *func, void *parm1, void *parm2)
    {
            while (1)
            {
                if (r->form)
                {
                    ((void (*)(rule_t *, void *, void *))func)
                        (r->rule [0], parm1, parm2);
                    break;
                }
                else
                {
                    ((void (*)(rule_t *, void *, void *))func)
                        (r->rule [2], parm1, parm2);

                    r = r->rule [0];
                }
            }
    }

    /* Converts the rule into an string. */
    void RuleToString (rule_t *r, char *dest)
    {
        int i;

            if (!r) return;

            for (i = 0; i < r->count; i++)
            {
                if (r->value [i])
                    strcpy (dest, r->value [i]);
                else
                    RuleToString (r->rule [i], dest);

                dest += strlen (dest);

                if (i + 1 < r->count)
                {
                    *dest++ = 0x20;
                    *dest = '\0';
                }
            }
    }
