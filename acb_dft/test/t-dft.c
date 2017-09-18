/*
    Copyright (C) 2016 Pascal Molin

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_dft.h"

typedef void (*do_f) (acb_ptr w, acb_srcptr v, slong len, slong prec);

void
check_vec_eq_prec(acb_srcptr w1, acb_srcptr w2, slong len, slong prec, slong digits, ulong q, char f1[], char f2[])
{
    slong i;

    for (i = 0; i < len; i++)
    {
        if (!acb_overlaps(w1 + i, w2 + i))
        {
            flint_printf("FAIL\n\n");
            flint_printf("q = %wu, size = %wu\n", q, len);
            flint_printf("\nDFT differ from index %ld / %ld \n", i, len);
            flint_printf("\n%s =\n", f1);
            acb_vec_printd_index(w1, len, digits);
            flint_printf("\n%s =\n", f2);
            acb_vec_printd_index(w2, len, digits);
            flint_printf("\n\n");
            abort();
        }
        else if (acb_rel_accuracy_bits(w1 + i) < 30
                || acb_rel_accuracy_bits(w2 + i) < 30)
        {
            flint_printf("FAIL\n\n");
            flint_printf("q = %wu\n", q);
            flint_printf("\nDFT inaccurate from index %ld / %ld \n", i, len);
            flint_printf("\nnaive =\n");
            acb_printd(w1 + i, digits);
            flint_printf("\nfast =\n");
            acb_printd(w2 + i, digits);
            flint_printf("\nerrors %ld & %ld [prec = %wu]\n",
                    acb_rel_accuracy_bits(w1 + i),
                    acb_rel_accuracy_bits(w2 + i), prec);
            abort();
        }
    }
}


int main()
{

    slong k;
    slong prec = 100, digits = 30;
    slong nq = 13;
    ulong q[13] = { 2, 3, 4, 5, 6, 23, 10, 15, 30, 59, 308, 335, 961};
    flint_rand_t state;

    slong f, nf = 3;
    do_f func[4] = { acb_dft_pol, acb_dft_cyc, acb_dft_crt , acb_dft_bluestein };
    char * name[4] = { "pol", "cyc", "crt", "bluestein" };

    flint_printf("dft....");
    fflush(stdout);

    flint_randinit(state);

    /* cyclic dft */
    for (k = 0; k < nq; k++)
    {
        slong i;
        acb_ptr v, w1, w2;

        v = _acb_vec_init(q[k]);
        w1 = _acb_vec_init(q[k]);
        w2 = _acb_vec_init(q[k]);

        for (i = 0; i < q[k]; i++)
            acb_set_si(v + i, i);

        for (f = 0; f < nf; f++)
        {

            acb_ptr w = (f == 0) ? w1 : w2;

            func[f](w, v, q[k], prec);

            if (f == 0)
                continue;

            check_vec_eq_prec(w1, w2, q[k], prec, digits, q[k], name[0], name[f]);

        }

        _acb_vec_clear(v, q[k]);
        _acb_vec_clear(w1, q[k]);
        _acb_vec_clear(w2, q[k]);
    }

    /* radix2 dft */
    for (k = 1; k < 12; k++)
    {
        slong n = 1 << k, j;
        acb_ptr v, w1, w2;
        v = w2 = _acb_vec_init(n);
        w1 = _acb_vec_init(n);

        for (j = 0; j < n; j++)
            acb_set_si(v + k, k);

        acb_dft_pol(w1, v, n, prec);
        acb_dft_rad2(v, k, prec);

        check_vec_eq_prec(w1, w2, n, prec, digits, n, "pol", "rad2");

    }

    flint_randclear(state);
    flint_cleanup();
    flint_printf("PASS\n");
    return EXIT_SUCCESS;
}
