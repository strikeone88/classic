/*
    SSTRING.C

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <sstring.h>

    /* String (set item) structure. */
    typedef struct
    {
        item_t  item;
        char    *value;
    }
    string_t;

    /* Shared strings set. */
    static set_t *set;

    /* String comparator. */
    static int cmpString (string_t *p, string_t *q)
    {
            return strcmp (p->value, q->value);
    }

    /* String destroyer. */
    static void kilString (string_t *p)
    {
            delete (p->value);
            delete (p);
    }

    /* Initializes the module. */
    void iString (void)
    {
            set = new (set_t);

            set->compare = (comparator_t)cmpString;
            set->destroy = (walker_t)kilString;
    }

    /* Deinitializes the module. */
    void diString (void)
    {
        string_t *p;

            while ((p = list__extract (set)) != NULL)
                kilString (p);
    }

    /* Adds a string. */
    char *aString (char *value)
    {
        string_t *p, *q;

            p = new (string_t);
            p->value = value;

            if ((q = set__search (set, p)) == NULL)
            {
                p->value = dups (value);
                set__addn (set, p);

                return p->value;
            }
            else
            {
                delete (p);
                set__hit (set, q);
                return q->value;
            }
    }

    /* Destroys a string. */
    void dString (char *value)
    {
        string_t p, *q;

            p.value = value;
            q = set__search (set, &p);

            set__destroy (set, q);
    }

    /* Adds a string (returns node). */
    StrNode *aStringN (char *value)
    {
        StrNode *p = new (StrNode);

            p->value = aString (value);
            return p;
    }

    /* Destroys a string node. */
    void dStringN (StrNode *p)
    {
            if (!p) return;

            dString (p->value);
            delete (p);
    }
