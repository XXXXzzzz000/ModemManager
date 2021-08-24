/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2010 Red Hat, Inc.
 * Copyright (C) 2011 The Chromium OS Authors.
 */

#include <glib.h>
#include <glib-object.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>

#define _LIBMM_INSIDE_MM
#include <libmm-glib.h>

#include "mm-sms-part-3gpp.h"
#include "mm-log-test.h"
#define CUSTOMER_PDU 1
/********************* PDU PARSER TESTS *********************/

static void
common_test_part_from_hexpdu (const gchar *hexpdu,
                              const gchar *expected_smsc,
                              const gchar *expected_number,
                              const gchar *expected_timestamp,
                              gboolean expected_multipart,
                              const gchar *expected_text,
                              const guint8 *expected_data,
                              gsize expected_data_size)
{
    MMSmsPart *part;
    GError *error = NULL;

    part = mm_sms_part_3gpp_new_from_pdu (0, hexpdu, NULL, &error);
    g_assert_no_error (error);
    g_assert (part != NULL);

    if (expected_smsc)
        g_assert_cmpstr (expected_smsc, ==, mm_sms_part_get_smsc (part));
    if (expected_number)
        g_assert_cmpstr (expected_number, ==, mm_sms_part_get_number (part));
    if (expected_timestamp)
        g_assert_cmpstr (expected_timestamp, ==, mm_sms_part_get_timestamp (part));
    if (expected_text)
        g_assert_cmpstr (expected_text, ==, mm_sms_part_get_text (part));
    g_assert_cmpuint (expected_multipart, ==, mm_sms_part_should_concat (part));

    if (expected_data) {
        guint32 i;
        const GByteArray *data;

        data = mm_sms_part_get_data (part);
        g_assert_cmpuint ((guint)expected_data_size, ==, data->len);
        for (i = 0; i < data->len; i++)
            g_assert_cmpuint ((guint)data->data[i], ==, expected_data[i]);
    }

    mm_sms_part_free (part);
}

static void
common_test_part_from_pdu (const guint8 *pdu,
                           gsize pdu_size,
                           const gchar *expected_smsc,
                           const gchar *expected_number,
                           const gchar *expected_timestamp,
                           gboolean expected_multipart,
                           const gchar *expected_text,
                           const guint8 *expected_data,
                           gsize expected_data_size)
{
    gchar *hexpdu;

    hexpdu = mm_utils_bin2hexstr (pdu, pdu_size);
    common_test_part_from_hexpdu (hexpdu,
                                  expected_smsc,
                                  expected_number,
                                  expected_timestamp,
                                  expected_multipart,
                                  expected_text,
                                  expected_data,
                                  expected_data_size);
    g_free (hexpdu);
}

static void
test_pdu1 (void)
{
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x04, 0x44, 0x29, 0x61, 0xf4,
        0x04, 0x0b, 0x91, 0x61, 0x71, 0x95, 0x72, 0x91,
        0xf8, 0x00, 0x00, 0x11, 0x20, 0x82, 0x11, 0x05,
        0x05, 0x0a,
        // user data:
        0x6a, 0xc8, 0xb2, 0xbc, 0x7c, 0x9a, 0x83, 0xc2,
        0x20, 0xf6, 0xdb, 0x7d, 0x2e, 0xcb, 0x41, 0xed,
        0xf2, 0x7c, 0x1e, 0x3e, 0x97, 0x41, 0x1b, 0xde,
        0x06, 0x75, 0x4f, 0xd3, 0xd1, 0xa0, 0xf9, 0xbb,
        0x5d, 0x06, 0x95, 0xf1, 0xf4, 0xb2, 0x9b, 0x5c,
        0x26, 0x83, 0xc6, 0xe8, 0xb0, 0x3c, 0x3c, 0xa6,
        0x97, 0xe5, 0xf3, 0x4d, 0x6a, 0xe3, 0x03, 0xd1,
        0xd1, 0xf2, 0xf7, 0xdd, 0x0d, 0x4a, 0xbb, 0x59,
        0xa0, 0x79, 0x7d, 0x8c, 0x06, 0x85, 0xe7, 0xa0,
        0x00, 0x28, 0xec, 0x26, 0x83, 0x2a, 0x96, 0x0b,
        0x28, 0xec, 0x26, 0x83, 0xbe, 0x60, 0x50, 0x78,
        0x0e, 0xba, 0x97, 0xd9, 0x6c, 0x17 };

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12404492164", /* smsc */
        "+16175927198", /* number */
        "2011-02-28T11:50:50-05:00", /* timestamp */
        FALSE,
        "Here's a longer message [{with some extended characters}] "
        "thrown in, such as £ and ΩΠΨ and §¿ as well.", /* text */
        NULL, 0);
}

static void
test_pdu2 (void)
{
    static const guint8 pdu[] = {
        0x07, 0x91, 0x97, 0x30, 0x07, 0x11, 0x11, 0xf1,
        0x04, 0x14, 0xd0, 0x49, 0x37, 0xbd, 0x2c, 0x77,
        0x97, 0xe9, 0xd3, 0xe6, 0x14, 0x00, 0x08, 0x11,
        0x30, 0x92, 0x91, 0x02, 0x40, 0x61, 0x08, 0x04,
        0x42, 0x04, 0x35, 0x04, 0x41, 0x04, 0x42};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+79037011111", /* smsc */
        "InternetSMS", /* number */
        "2011-03-29T19:20:04+04:00", /* timestamp */
        FALSE,
        "тест", /* text */
        NULL, 0);
}

static void
test_pdu3 (void)
{
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0x00, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "+18005551212", /* number */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        "hellohello", /* text */
        NULL, 0);
}

static void
test_pdu3_nzpid (void)
{
    /* pid is nonzero (00 -> ff) */
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0xff, 0x00, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "+18005551212", /* number */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        "hellohello", /* text */
        NULL, 0);
}

static void
test_pdu3_mms (void)
{
    /* mms is clear (04 -> 00) */
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x00, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0x00, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "+18005551212", /* number */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        "hellohello", /* text */
        NULL, 0);
}

static void
test_pdu3_natl (void)
{
    /* number is natl (91 -> 81) */
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x81, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0x00, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "18005551212", /* number, no plus */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        "hellohello", /* text */
        NULL, 0);
}

static void
test_pdu3_8bit (void)
{
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0x04, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37, 0xde};
    static const guint8 expected_data[] = {
        0xe8, 0x32, 0x9b, 0xfd, 0x46, 0x97, 0xd9, 0xec, 0x37, 0xde };

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "+18005551212", /* number */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        NULL, /* text */
        expected_data, /* data */
        sizeof (expected_data)); /* data size */
}

static void
test_pdu_dcsf1 (void)
{
    /* TP-DCS coding scheme is group F */
    static const guint8 pdu[] = {
        0x07,       // length of SMSC info
        0x91,       // type of address of SMSC (E.164)
        0x33, 0x06, 0x09, 0x10, 0x93, 0xF0,  // SMSC address (+33 60 90 01 39 0)
        0x04,       // SMS-DELIVER
        0x04,       // address length
        0x85,       // type of address
        0x81, 0x00, // sender address (1800)
        0x00,       // TP-PID protocol identifier
        0xF1,       // TP-DCS data coding scheme
        0x11, 0x60, 0x42, 0x31, 0x80, 0x51, 0x80,   // timestamp 11-06-24 13:08:51
        0xA0,       // TP-UDL user data length (160)
        // Content:
        0x49,
        0xB7, 0xF9, 0x0D, 0x9A, 0x1A, 0xA5, 0xA0, 0x16,
        0x68, 0xF8, 0x76, 0x9B, 0xD3, 0xE4, 0xB2, 0x9B,
        0x9E, 0x2E, 0xB3, 0x59, 0xA0, 0x3F, 0xC8, 0x5D,
        0x06, 0xA9, 0xC3, 0xED, 0x70, 0x7A, 0x0E, 0xA2,
        0xCB, 0xC3, 0xEE, 0x79, 0xBB, 0x4C, 0xA7, 0xCB,
        0xCB, 0xA0, 0x56, 0x43, 0x61, 0x7D, 0xA7, 0xC7,
        0x69, 0x90, 0xFD, 0x4D, 0x97, 0x97, 0x41, 0xEE,
        0x77, 0xDD, 0x5E, 0x0E, 0xD7, 0x41, 0xED, 0x37,
        0x1D, 0x44, 0x2E, 0x83, 0xE0, 0xE1, 0xF9, 0xBC,
        0x0C, 0xD2, 0x81, 0xE6, 0x77, 0xD9, 0xB8, 0x4C,
        0x06, 0xC1, 0xDF, 0x75, 0x39, 0xE8, 0x5C, 0x90,
        0x97, 0xE5, 0x20, 0xFB, 0x9B, 0x2E, 0x2F, 0x83,
        0xC6, 0xEF, 0x36, 0x9C, 0x5E, 0x06, 0x4D, 0x8D,
        0x52, 0xD0, 0xBC, 0x2E, 0x07, 0xDD, 0xEF, 0x77,
        0xD7, 0xDC, 0x2C, 0x77, 0x99, 0xE5, 0xA0, 0x77,
        0x1D, 0x04, 0x0F, 0xCB, 0x41, 0xF4, 0x02, 0xBB,
        0x00, 0x47, 0xBF, 0xDD, 0x65, 0x50, 0xB8, 0x0E,
        0xCA, 0xD9, 0x66};

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+33609001390", /* smsc */
        "1800", /* number */
        "2011-06-24T13:08:15+02:00", /* timestamp */
        FALSE,
        "Info SFR - Confidentiel, à ne jamais transmettre -\r\n"
        "Voici votre nouveau mot de passe : sw2ced pour gérer "
        "votre compte SFR sur www.sfr.fr ou par téléphone au 963", /* text */
        NULL, 0);
}

static void
test_pdu_dcsf_8bit (void)
{
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0xf4, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0a, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37, 0xde};
    static const guint8 expected_data[] = {
        0xe8, 0x32, 0x9b, 0xfd, 0x46, 0x97, 0xd9, 0xec, 0x37, 0xde };

    common_test_part_from_pdu (
        pdu, sizeof (pdu),
        "+12345678901", /* smsc */
        "+18005551212", /* number */
        "2011-01-01T12:34:56+00:00", /* timestamp */
        FALSE,
        NULL, /* text */
        expected_data, /* data */
        sizeof (expected_data)); /* data size */
}

static void
test_pdu_insufficient_data (void)
{
    GError *error = NULL;
    MMSmsPart *part;
    gchar *hexpdu;
    static const guint8 pdu[] = {
        0x07, 0x91, 0x21, 0x43, 0x65, 0x87, 0x09, 0xf1,
        0x04, 0x0b, 0x91, 0x81, 0x00, 0x55, 0x15, 0x12,
        0xf2, 0x00, 0x00, 0x11, 0x10, 0x10, 0x21, 0x43,
        0x65, 0x00, 0x0b, 0xe8, 0x32, 0x9b, 0xfd, 0x46,
        0x97, 0xd9, 0xec, 0x37
    };

    hexpdu = mm_utils_bin2hexstr (pdu, sizeof (pdu));
    part = mm_sms_part_3gpp_new_from_pdu (0, hexpdu, NULL, &error);
    g_assert (part == NULL);
    /* We don't care for the specific error type */
    g_assert (error != NULL);
    g_error_free (error);
    g_free (hexpdu);
}


static void
test_pdu_udhi (void)
{
    /* Welcome message from KPN NL */
    static const gchar *hexpdu =
        "07911356131313F64004850120390011609232239180A006080400100201D7327BFD6EB340E232"
        "1BF46E83EA7790F59D1E97DBE1341B442F83C465763D3DA797E56537C81D0ECB41AB59CC1693C1"
        "6031D96C064241E5656838AF03A96230982A269BCD462917C8FA4E8FCBED709A0D7ABBE9F6B0FB"
        "5C7683D27350984D4FABC9A0B33C4C4FCF5D20EBFB2D079DCB62793DBD06D9C36E50FB2D4E97D9"
        "A0B49B5E96BBCB";

    common_test_part_from_hexpdu (
        hexpdu,
        "+31653131316", /* smsc */
        "1002", /* number */
        "2011-06-29T23:32:19+02:00", /* timestamp */
        TRUE,
        "Welkom, bel om uw Voicemail te beluisteren naar +31612001233"
        " (PrePay: *100*1233#). Voicemail ontvangen is altijd gratis."
        " Voor gebruik van mobiel interne", /* text */
        NULL, 0);
}

static void
test_pdu_multipart (void)
{
    static const gchar *hexpdu1 =
        "07912160130320F5440B916171056429F5000021405291650569A00500034C0201A9E8F41C949E"
        "83C2207B599E07B1DFEE33885E9ED341E4F23C7D7697C920FA1B54C697E5E3F4BC0C6AD7D9F434"
        "081E96D341E3303C2C4EB3D3F4BC0B94A483E6E8779D4D06CDD1EF3BA80E0785E7A0B7BB0C6A97"
        "E7F3F0B9CC02B9DF7450780EA2DFDF2C50780EA2A3CBA0BA9B5C96B3F369F71954768FDFE4B4FB"
        "0C9297E1F2F2BCECA6CF41";
    static const gchar *hexpdu2 =
        "07912160130320F6440B916171056429F5000021405291651569320500034C0202E9E8301D4447"
        "9741F0B09C3E0785E56590BCCC0ED3CB6410FD0D7ABBCBA0B0FB4D4797E52E10";

    common_test_part_from_hexpdu (
        hexpdu1,
        "+12063130025", /* smsc */
        "+16175046925", /* number */
        "2012-04-25T19:56:50-04:00", /* timestamp */
        TRUE, /* multipart! */
        "This is a very long test designed to exercise multi part capability. It should "
        "show up as one message, not as two, as the underlying encoding represents ", /* text */
        NULL, 0);

    common_test_part_from_hexpdu (
        hexpdu2,
        "+12063130026", /* smsc */
        "+16175046925", /* number */
        "2012-04-25T19:56:51-04:00", /* timestamp */
        TRUE, /* multipart! */
        "that the parts are related to one another. ", /* text */
        NULL, 0);
}

static void
test_pdu_stored_by_us (void)
{
    /* This is a SUBMIT PDU! */
    static const gchar *hexpdu1 =
        "002100098136397339F70008224F60597D4F60597D4F60597D4F60597D4F60597D4F60597D4F60597D4F60597D4F60";

    common_test_part_from_hexpdu (
        hexpdu1,
        NULL, /* smsc */
        "639337937", /* number */
        NULL, /* timestamp */
        FALSE, /* multipart! */
        "你好你好你好你好你好你好你好你好你", /* text */
        NULL, 0);
}

static void
test_pdu_not_stored (void)
{
    static const gchar *hexpdu1 =
        "07914356060013F1065A098136397339F7219011700463802190117004638030";

    common_test_part_from_hexpdu (
        hexpdu1,
        "+34656000311", /* smsc */
        "639337937", /* number */
        "2012-09-11T07:40:36+02:00", /* timestamp */
        FALSE, /* multipart! */
        NULL, /* text */
        NULL, 0);
}

/********************* SMS ADDRESS ENCODER TESTS *********************/

static void
common_test_address_encode (const gchar *address,
                            gboolean smsc,
                            const guint8 *expected,
                            gsize expected_size)
{
    guint8 buf[20];
    gsize enclen;

    enclen = mm_sms_part_3gpp_encode_address (address, buf, sizeof (buf), smsc);
    g_assert_cmpuint (enclen, ==, expected_size);
    g_assert_cmpint (memcmp (buf, expected, expected_size), ==, 0);
}

static void
test_address_encode_smsc_intl (void)
{
    static const gchar *addr = "+19037029920";
    static const guint8 expected[] = { 0x07, 0x91, 0x91, 0x30, 0x07, 0x92, 0x29, 0xF0 };

    common_test_address_encode (addr, TRUE, expected, sizeof (expected));
}

static void
test_address_encode_smsc_unknown (void)
{
    static const char *addr = "9037029920";
    static const guint8 expected[] = { 0x06, 0x81, 0x09, 0x73, 0x20, 0x99, 0x02 };

    common_test_address_encode (addr, TRUE, expected, sizeof (expected));
}

static void
test_address_encode_intl (void)
{
    static const char *addr = "+19037029920";
    static const guint8 expected[] = { 0x0B, 0x91, 0x91, 0x30, 0x07, 0x92, 0x29, 0xF0 };

    common_test_address_encode (addr, FALSE, expected, sizeof (expected));
}

static void
test_address_encode_unknown (void)
{
    static const char *addr = "9037029920";
    static const guint8 expected[] = { 0x0A, 0x81, 0x09, 0x73, 0x20, 0x99, 0x02 };

    common_test_address_encode (addr, FALSE, expected, sizeof (expected));
}

/********************* PDU CREATOR TESTS *********************/

static void
trace_pdu (const guint8 *pdu,
           guint         len)
{
    guint i;

    g_print ("n        ");
    for (i = 0; i < len; i++) {
        g_print ("  0x%02X", pdu[i]);
        if (((i + 1) % 12) == 0)
            g_print ("n        ");
    }
    g_print ("n");
}

static void
common_test_create_pdu (const gchar *smsc,
                        const gchar *number,
                        const gchar *text,
                        guint validity,
                        gint  class,
                        const guint8 *expected,
                        gsize expected_size,
                        guint expected_msgstart)
{
    MMSmsPart *part;
    guint8 *pdu;
    guint len = 0, msgstart = 0;
    GError *error = NULL;

    part = mm_sms_part_new (0, MM_SMS_PDU_TYPE_SUBMIT);
    if (smsc)
        mm_sms_part_set_smsc (part, smsc);
    if (number)
        mm_sms_part_set_number (part, number);
    if (text) {
        gchar **out;
        MMSmsEncoding encoding = MM_SMS_ENCODING_UNKNOWN;

        /* Detect best encoding */
        out = mm_sms_part_3gpp_util_split_text (text, &encoding, NULL);
        g_strfreev (out);
        mm_sms_part_set_text (part, text);
        mm_sms_part_set_encoding (part, encoding);
    }
    if (validity > 0)
        mm_sms_part_set_validity_relative (part, validity);
    if (class >= 0)
        mm_sms_part_set_class (part, class);

    pdu = mm_sms_part_3gpp_get_submit_pdu (part,
                                           &len,
                                           &msgstart,
                                           NULL,
                                           &error);
    mm_sms_part_free (part);

    if (g_test_verbose ())
        trace_pdu (pdu, len);

    g_assert_no_error (error);
    g_assert (pdu != NULL);
    g_assert_cmpmem (pdu, len, expected, expected_size);
    g_assert_cmpint (msgstart, ==, expected_msgstart);

    g_free (pdu);
}

static void
test_create_pdu_ucs2_with_smsc (void)
{
    static const char *smsc = "+19037029920";
    static const char *number = "+15555551234";
    static const char *text = "Да здравствует король, детка!";
    static const guint8 expected[] = {
        0x07, 0x91, 0x91, 0x30, 0x07, 0x92, 0x29, 0xF0, 0x11, 0x00, 0x0B, 0x91,
        0x51, 0x55, 0x55, 0x15, 0x32, 0xF4, 0x00, 0x08, 0x00, 0x3A, 0x04, 0x14,
        0x04, 0x30, 0x00, 0x20, 0x04, 0x37, 0x04, 0x34, 0x04, 0x40, 0x04, 0x30,
        0x04, 0x32, 0x04, 0x41, 0x04, 0x42, 0x04, 0x32, 0x04, 0x43, 0x04, 0x35,
        0x04, 0x42, 0x00, 0x20, 0x04, 0x3A, 0x04, 0x3E, 0x04, 0x40, 0x04, 0x3E,
        0x04, 0x3B, 0x04, 0x4C, 0x00, 0x2C, 0x00, 0x20, 0x04, 0x34, 0x04, 0x35,
        0x04, 0x42, 0x04, 0x3A, 0x04, 0x30, 0x00, 0x21
    };

    common_test_create_pdu (smsc,
                            number,
                            text,
                            5, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            8); /* expected_msgstart */
}

static void
test_create_pdu_ucs2_no_smsc (void)
{
    static const char *number = "+15555551234";
    static const char *text = "Да здравствует король, детка!";
    static const guint8 expected[] = {
        0x00, 0x11, 0x00, 0x0B, 0x91, 0x51, 0x55, 0x55, 0x15, 0x32, 0xF4, 0x00,
        0x08, 0x00, 0x3A, 0x04, 0x14, 0x04, 0x30, 0x00, 0x20, 0x04, 0x37, 0x04,
        0x34, 0x04, 0x40, 0x04, 0x30, 0x04, 0x32, 0x04, 0x41, 0x04, 0x42, 0x04,
        0x32, 0x04, 0x43, 0x04, 0x35, 0x04, 0x42, 0x00, 0x20, 0x04, 0x3A, 0x04,
        0x3E, 0x04, 0x40, 0x04, 0x3E, 0x04, 0x3B, 0x04, 0x4C, 0x00, 0x2C, 0x00,
        0x20, 0x04, 0x34, 0x04, 0x35, 0x04, 0x42, 0x04, 0x3A, 0x04, 0x30, 0x00,
        0x21
    };

    common_test_create_pdu (NULL, /* smsc */
                            number,
                            text,
                            5, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            1); /* expected_msgstart */
}

static void
test_create_pdu_gsm_with_smsc (void)
{
    static const char *smsc = "+19037029920";
    static const char *number = "+15555551234";
    static const char *text = "Hi there...Tue 17th Jan 2012 05:30.18 pm (GMT+1) ΔΔΔΔΔ";
    static const guint8 expected[] = {
        0x07, 0x91, 0x91, 0x30, 0x07, 0x92, 0x29, 0xF0, 0x11, 0x00, 0x0B, 0x91,
        0x51, 0x55, 0x55, 0x15, 0x32, 0xF4, 0x00, 0x00, 0x00, 0x36, 0xC8, 0x34,
        0x88, 0x8E, 0x2E, 0xCB, 0xCB, 0x2E, 0x97, 0x8B, 0x5A, 0x2F, 0x83, 0x62,
        0x37, 0x3A, 0x1A, 0xA4, 0x0C, 0xBB, 0x41, 0x32, 0x58, 0x4C, 0x06, 0x82,
        0xD5, 0x74, 0x33, 0x98, 0x2B, 0x86, 0x03, 0xC1, 0xDB, 0x20, 0xD4, 0xB1,
        0x49, 0x5D, 0xC5, 0x52, 0x20, 0x08, 0x04, 0x02, 0x81, 0x00
    };

    common_test_create_pdu (smsc,
                            number,
                            text,
                            5, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            8); /* expected_msgstart */
}

static void
test_create_pdu_gsm_no_smsc (void)
{
    static const char *number = "+15555551234";
    static const char *text = "Hi there...Tue 17th Jan 2012 05:30.18 pm (GMT+1) ΔΔΔΔΔ";
    static const guint8 expected[] = {
        0x00, 0x11, 0x00, 0x0B, 0x91, 0x51, 0x55, 0x55, 0x15, 0x32, 0xF4, 0x00,
        0x00, 0x00, 0x36, 0xC8, 0x34, 0x88, 0x8E, 0x2E, 0xCB, 0xCB, 0x2E, 0x97,
        0x8B, 0x5A, 0x2F, 0x83, 0x62, 0x37, 0x3A, 0x1A, 0xA4, 0x0C, 0xBB, 0x41,
        0x32, 0x58, 0x4C, 0x06, 0x82, 0xD5, 0x74, 0x33, 0x98, 0x2B, 0x86, 0x03,
        0xC1, 0xDB, 0x20, 0xD4, 0xB1, 0x49, 0x5D, 0xC5, 0x52, 0x20, 0x08, 0x04,
        0x02, 0x81, 0x00
    };

    common_test_create_pdu (NULL, /* smsc */
                            number,
                            text,
                            5, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            1); /* expected_msgstart */
}

static void
test_create_pdu_gsm_3 (void)
{
    static const char *number = "+15556661234";
    static const char *text = "This is really cool ΔΔΔΔΔ";
    static const guint8 expected[] = {
        0x00, 0x11, 0x00, 0x0B, 0x91, 0x51, 0x55, 0x66, 0x16, 0x32, 0xF4, 0x00,
        0x00, 0x00, 0x19, 0x54, 0x74, 0x7A, 0x0E, 0x4A, 0xCF, 0x41, 0xF2, 0x72,
        0x98, 0xCD, 0xCE, 0x83, 0xC6, 0xEF, 0x37, 0x1B, 0x04, 0x81, 0x40, 0x20,
        0x10
    };

    /* Tests that a 25-character message (where the last septet is packed into
     * an octet by itself) is created correctly.  Previous to
     * "core: fix some bugs in GSM7 packing code" the GSM packing code would
     * leave off the last octet.
     */

    common_test_create_pdu (NULL, /* smsc */
                            number,
                            text,
                            5, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            1); /* expected_msgstart */
}

static void
test_create_pdu_gsm_no_validity (void)
{
    static const char *number = "+15556661234";
    static const char *text = "This is really cool ΔΔΔΔΔ";
    static const guint8 expected[] = {
        0x00, 0x01, 0x00, 0x0B, 0x91, 0x51, 0x55, 0x66, 0x16, 0x32, 0xF4, 0x00,
        0x00, 0x19, 0x54, 0x74, 0x7A, 0x0E, 0x4A, 0xCF, 0x41, 0xF2, 0x72, 0x98,
        0xCD, 0xCE, 0x83, 0xC6, 0xEF, 0x37, 0x1B, 0x04, 0x81, 0x40, 0x20, 0x10
    };

    common_test_create_pdu (NULL, /* smsc */
                            number,
                            text,
                            0, /* validity */
                            -1, /* class */
                            expected,
                            sizeof (expected),
                            1); /* expected_msgstart */
}

/********************* TEXT SPLIT TESTS *********************/

static void
common_test_text_split (const gchar *text,
                        const gchar **expected,
                        MMSmsEncoding expected_encoding)
{
    gchar **out;
    MMSmsEncoding out_encoding = MM_SMS_ENCODING_UNKNOWN;
    guint i;

    out = mm_sms_part_3gpp_util_split_text (text, &out_encoding, NULL);

    g_assert (out != NULL);
    g_assert (out_encoding != MM_SMS_ENCODING_UNKNOWN);

    g_assert_cmpuint (g_strv_length (out), ==, g_strv_length ((gchar **)expected));

    for (i = 0; out[i]; i++) {
        g_assert_cmpstr (out[i], ==, expected[i]);
    }

    g_strfreev (out);
}

static void
test_text_split_short_gsm7 (void)
{
    const gchar *text = "Hello";
    const gchar *expected [] = {
        "Hello",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_GSM7);
}

static void
test_text_split_short_ucs2 (void)
{
    const gchar *text = "你好"; /* (UTF-8) e4 bd a0 e5 a5 bd */
    const gchar *expected [] = {
        "你好",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}

static void
test_text_split_short_utf16 (void)
{
    const gchar *text = "😉"; /* U+1F609, winking face */
    const gchar *expected [] = {
        "😉",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}

static void
test_text_split_max_single_pdu_gsm7 (void)
{
    const gchar *text =
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789";
    const gchar *expected [] = {
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_GSM7);
}

static void
test_text_split_max_single_pdu_ucs2 (void)
{
    /* NOTE: This chinese string contains 210 bytes when encoded in
     * UTF-8! But still, it can be placed into 140 bytes when in UCS-2
     */
    const gchar *text =
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好";
    const gchar *expected [] = {
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}

static void
test_text_split_max_single_pdu_utf16 (void)
{
    /* NOTE: this string contains 35 Bhaiksuki characters, each of
     * them requiring 4 bytes both in UTF-8 and in UTF-16 (140 bytes
     * in total). */
    const gchar *text =
        "𑰀𑰁𑰂𑰃𑰄𑰅𑰆𑰇𑰈𑰊𑰋𑰌𑰍𑰎𑰏𑰐𑰑𑰒𑰓𑰔𑰕𑰖𑰗𑰘𑰙𑰚𑰛𑰜𑰝𑰞𑰟𑰠𑰡𑰢𑰣";
    const gchar *expected [] = {
        "𑰀𑰁𑰂𑰃𑰄𑰅𑰆𑰇𑰈𑰊𑰋𑰌𑰍𑰎𑰏𑰐𑰑𑰒𑰓𑰔𑰕𑰖𑰗𑰘𑰙𑰚𑰛𑰜𑰝𑰞𑰟𑰠𑰡𑰢𑰣",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}

static void
test_text_split_two_pdu_gsm7 (void)
{
    const gchar *text =
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890";
    const gchar *expected [] = {
        /* First chunk */
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "012345678901234567890123456789012",
        /* Second chunk */
        "34567890",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_GSM7);
}

static void
test_text_split_two_pdu_ucs2 (void)
{
    const gchar *text =
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好好";
    const gchar *expected [] = {
        /* First chunk */
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好"
        "你好你",
        /* Second chunk */
        "好你好好",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}

static void
test_text_split_two_pdu_utf16 (void)
{
    /* NOTE: this string contains 35 Bhaiksuki characters, each of
     * them requiring 4 bytes both in UTF-8 and in UTF-16 (140 bytes
     * in total) plus one ASCII char (encoded with 1 byte in UTF-8 and
     * 2 bytes in UTF-16), making it a total of 142 bytes when in
     * UTF-16 (so not fitting in one single PDU)
     *
     * When split in chunks, the last chunk will hold 2 Bhaiksuki
     * characters plus the last ASCII one (9 bytes in UTF-16) so that
     * the first chunk contains the leading 33 Bhaiksuki characters
     * (132 characters, less than 134) */
    const gchar *text =
        "𑰀𑰁𑰂𑰃𑰄𑰅𑰆𑰇𑰈𑰊𑰋𑰌𑰍𑰎𑰏𑰐𑰑𑰒𑰓𑰔𑰕𑰖𑰗𑰘𑰙𑰚𑰛𑰜𑰝𑰞𑰟𑰠𑰡𑰢𑰣a";
    const gchar *expected [] = {
        "𑰀𑰁𑰂𑰃𑰄𑰅𑰆𑰇𑰈𑰊𑰋𑰌𑰍𑰎𑰏𑰐𑰑𑰒𑰓𑰔𑰕𑰖𑰗𑰘𑰙𑰚𑰛𑰜𑰝𑰞𑰟𑰠𑰡",
        "𑰢𑰣a",
        NULL
    };

    common_test_text_split (text, expected, MM_SMS_ENCODING_UCS2);
}
struct test_info
{
   const gchar *hexpdu;
   const gchar *expected_smsc;
   const gchar *expected_number;
   const gchar *expected_timestamp;
    gboolean expected_multipart;
   const gchar *expected_text;
   const guint8 *expected_data;
    gsize expected_data_size;
};
static void
show_part_info(MMSmsPart *part)
{
    MMSmsPduType pdu_type;
    const gchar *smsc;
    const gchar *number;
    const gchar *timestamp;
    const gchar *text;
    MMSmsEncoding encoding;
    gint class;
    guint validity_relative;
    gboolean delivery_report_request;
    guint concat_reference;
    guint concat_max;
    guint concat_sequence;
    const GByteArray *data;

    pdu_type = mm_sms_part_get_pdu_type(part);
    g_print("pdu type: %s\n", mm_sms_pdu_type_get_string(pdu_type));

    smsc = mm_sms_part_get_smsc(part);
    g_print("smsc: %s\n", smsc ? smsc : "n/a");

    number = mm_sms_part_get_number(part);
    g_print("number: %s\n", number ? number : "n/a");

    timestamp = mm_sms_part_get_timestamp(part);
    g_print("timestamp: %s\n", timestamp ? timestamp : "n/a");

    encoding = mm_sms_part_get_encoding(part);
    switch (encoding)
    {
    case MM_SMS_ENCODING_GSM7:
        g_print("encoding: GSM7\n");
        break;
    case MM_SMS_ENCODING_UCS2:
        g_print("encoding: UCS2\n");
        break;
    case MM_SMS_ENCODING_8BIT:
        g_print("encoding: 8BIT\n");
        break;
    case MM_SMS_ENCODING_UNKNOWN:
    default:
        g_print("encoding: unknown (0x%x)\n", encoding);
        break;
    }

    text = mm_sms_part_get_text(part);
    g_print("text: %s\n", text ? text : "n/a");

    data = mm_sms_part_get_data(part);
    if (data)
    {
        gchar *data_str;

        data_str = mm_utils_bin2hexstr(data->data, data->len);
        g_print("data: %s\n", data_str);
        g_free(data_str);
    }
    else
        g_print("data: n/a\n");

    class = mm_sms_part_get_class(part);
    if (class != -1)
        g_print("class: %d\n", class);
    else
        g_print("class: n/a\n");

    validity_relative = mm_sms_part_get_validity_relative(part);
    if (validity_relative != 0)
        g_print("validity relative: %d\n", validity_relative);
    else
        g_print("validity relative: n/a\n");

    delivery_report_request = mm_sms_part_get_delivery_report_request(part);
    g_print("delivery report request: %s\n", delivery_report_request ? "yes" : "no");

    concat_reference = mm_sms_part_get_concat_reference(part);
    g_print("concat reference: %d\n", concat_reference);

    concat_max = mm_sms_part_get_concat_max(part);
    g_print("concat max: %d\n", concat_max);

    concat_sequence = mm_sms_part_get_concat_sequence(part);
    g_print("concat sequence: %d\n", concat_sequence);

    if (mm_sms_part_get_pdu_type(part) == MM_SMS_PDU_TYPE_STATUS_REPORT)
    {
        const gchar *discharge_timestamp;
        guint message_reference;
        guint delivery_state;

        message_reference = mm_sms_part_get_message_reference(part);
        g_print("message reference: %d\n", message_reference);

        discharge_timestamp = mm_sms_part_get_discharge_timestamp(part);
        g_print("discharge timestamp: %s\n", discharge_timestamp ? discharge_timestamp : "n/a");

        delivery_state = mm_sms_part_get_delivery_state(part);
        g_print("delivery state: %s\n", mm_sms_delivery_state_get_string_extended(delivery_state));
    }

    if (MM_SMS_PART_IS_CDMA(part))
    {
        MMSmsCdmaTeleserviceId teleservice_id;
        MMSmsCdmaServiceCategory service_category;

        teleservice_id = mm_sms_part_get_cdma_teleservice_id(part);
        g_print("teleservice id: %s\n", mm_sms_cdma_teleservice_id_get_string(teleservice_id));

        service_category = mm_sms_part_get_cdma_service_category(part);
        g_print("service category: %s\n", mm_sms_cdma_service_category_get_string(service_category));
    }
}
#if CUSTOMER_PDU
static void
test_askey_pdu_china_unicom(void)
{
    int i = 0;
    const gchar *hexpdu[] = {
        "6405a00110f0000812305251638523880500030504015c0a656c768475286237ff0c62"
        "2a81f35f53524dff0c60a876846d4191cf4f7f752860c551b559824e0bff1a000d000a"
        "0020002d5f536708514d8d396d4191cf00350031002e00350035004d0042000d000a00"
        "20002d5f5367085957991059166d4191cf60c551b5000d000a002056fd518565e579df"
        "5b9d000d000a00205df24f7f",
        "6405a00110f000081230525173302388050003050402752800370030002e0030003000\
4d0042000d000a0020000d000a0020672c6570636e4ec54f9b53c28003ff0c53ef4e0b\
8f7d624b673a84254e1a538567e58be28be67ec660c551b53002000d000a002056de59\
0d201c0035003000380033201dff0c67e5770b6d4191cf534a5e7453054f5991cf3002\
000d000a002056de590d201c",
        "6405a00110f0000812305251739023880500030504030032003000380032201dff0c67\
e5770b5f53524d595799104f5991cf3002000d000a00203010629751fb75ab60c5ff0c\
670d52a14e0d505cff014f7f7528624b673a84254e1a5385ff0c8db34e0d51fa62374e\
a48bdd8d39300167e54f59989d3001529e4e1a52a1ff0c514d6d4191cf770b75355f71\
300173a96e38620fff0c70b9",
        "6405a00110f00008123052517341234605000305040451fb0020006800740074007000\
3a002f002f0075002e00310030003000310030002e0063006e002f006b006800640064\
00780020ff0c9a6c4e0a62e567093011",
        NULL,
    };
    struct test_info info[] = {
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10010",                     /* number */
            "2021-03-25T15:36:58+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "尊敬的用户，截至当前，您的流量使用情况如下：\r\n"
            " -当月免费流量51.55MB\r\n"
            " -当月套餐外流量情况\r\n"
            " 国内日租宝\r\n"
            " 已使", /* text */
            NULL,    /* expected data */
            0        /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10010",                     /* number */
            "2021-03-25T15:37:03+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "用70.00MB\r\n "
            "\r\n"
            " 本数据仅供参考，可下载手机营业厅查询详细情况。\r\n"
            " 回复“5083”，查看流量半年包余量。\r\n"
            " 回复“", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10010",                     /* number */
            "2021-03-25T15:37:09+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "2082”，查看当前套餐余量。\r\n"
" 【抗击疫情，服务不停！使用手机营业厅，足不出户交话费、查余额、办业务，免流量看电影、玩游戏，点", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10010",                     /* number */
            "2021-03-25T15:37:14+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "击 http://u.10010.cn/khddx ，马上拥有】", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
    };
    #if 0
    (void)info;

    for (i=0; hexpdu[i] != NULL; i++)
    {
        GError *error = NULL;
        MMSmsPart *part;
        g_print("index %d=============================", i);
        part = mm_sms_part_3gpp_new_from_pdu(0, hexpdu[i], NULL, &error);
        g_assert_no_error(error);
        show_part_info(part);
        mm_sms_part_free(part);
    }
    #else
    (void)show_part_info;
    for (i=0; hexpdu[i] != NULL; i++)
    {
        info[i].hexpdu=hexpdu[i];
        g_print("index %d=============================", i);
        common_test_part_from_hexpdu(
            info[i].hexpdu,
            info[i].expected_smsc,
            info[i].expected_number,
            info[i].expected_timestamp,
            info[i].expected_multipart,
            info[i].expected_text,
            info[i].expected_data,
            info[i].expected_data_size);
    }
    #endif
}
static void
test_askey_pdu_A1(void)
{
    int i = 0;
    const gchar *hexpdu[] = {
        "040a81604696401200001230520174124096cc74595c96838231d0b2ee269759a069ba"
"0c4287c56537685e4ed34141f17c8c66d7e77350122d2fcb41ec325d4f2fbb41d2f218"
"edaebbcfa071d805a2d56835578e066a0a41c430bdecb6bfd9f576d90db297e56279b8"
"3e46d35da0a0fb1c1697dda079da4d06bdd1ee32e858beefd17217288996838231d0b4"
"2cb7a7c76510b51c6e03",
"040a8160469640120000123052017412409bcc74595c96838231d0b2ee269759a0245a"
"5e0659cb7271da4daebbcff3b29b7e2eb3e965d0bc9ca68382e2f918cdaecfe7a0245a"
"5e9683d865ba9e5e7683a4e531da5d779f41e2325d1e3e97dda071d8052a56a5a0990b"
"060389e5753afded0205dde7b0b8ec06cdd36e32e88d769741c7f27d8f96bb4049b41c"
"148c81a665b93d3d2e83a8e5701b",
        NULL,
    };
    struct test_info info[] = {
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "0664690421",                     /* number */
            "2021-03-25T10:47:21+01:00", /* timestamp */
            FALSE,                        /* expected multipart */
            "Lieber A1 Kunde, Sie haben seit Abschluss Ihrer letzten Rechnung ca. 4545.94 MB Datenvolumen verbraucht. Angaben sind ohne Gewähr. Ihr A1 Service Team", /* text */
            NULL,    /* expected data */
            0        /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "0664690421",                     /* number */
            "2021-03-25T10:47:21+01:00", /* timestamp */
            FALSE,                        /* expected multipart */
            "Lieber A1 Kunde, Ihre Verbindungsentgelte seit Abschluss Ihrer letzten Rechnung betragen ca. EUR 3.00 brutto. Angaben sind ohne Gewähr. Ihr A1 Service Team", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
    };
    #if 0
    (void)info;

    for (i=0; hexpdu[i] != NULL; i++)
    {
        GError *error = NULL;
        MMSmsPart *part;
        g_print("index %d=============================", i);
        part = mm_sms_part_3gpp_new_from_pdu(0, hexpdu[i], NULL, &error);
        g_assert_no_error(error);
        show_part_info(part);
        mm_sms_part_free(part);
    }
    #else
    (void)show_part_info;
    for (i=0; hexpdu[i] != NULL; i++)
    {
        info[i].hexpdu=hexpdu[i];
        g_print("index %d=============================", i);
        common_test_part_from_hexpdu(
            info[i].hexpdu,
            info[i].expected_smsc,
            info[i].expected_number,
            info[i].expected_timestamp,
            info[i].expected_multipart,
            info[i].expected_text,
            info[i].expected_data,
            info[i].expected_data_size);
    }
    #endif
}
static void
test_askey_pdu_china_telecom(void)
{
    int i = 0;
    const gchar *hexpdu[] = {
       "2405a10100f1000812305281113023745c0a656c76846df1002a5e02002a6cfd002a4e"
"1a002a7406002a9650002a53f85ba26237ff1a60a85f53524d5df24f7f75286d4191cf"
"603b91cf00330036002e0032003800470042ff1b8be67ec6595799106d4191cf4f7f75"
"2860c551b553ef56de590d201c003700300032201d67e58be23002",
/* need contact */
"6405a10100f1000812305281219523880500034302015c0a656c76846df1002a5e0200"
"2a6cfd002a4e1a002a7406002a9650002a53f85ba26237ff1a622a6b6281f300326708"
"0032003865e5ff0c60a87684624b673a8d266237672c670853ef75284f59989d4e3a00"
"30002e003000305143ff0c98845b588d3975284f59989d4e3a0030002e00300030ff0c"
"8d6090018d3975284f59989d",
"6405a10100f1000812305281310023200500034302024e3a0030002e00300030300261"
"1f8c2260a876844f7f7528ff01",
        NULL,
    };
    struct test_info info[] = {
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10001",                     /* number */
            "2021-03-25T18:11:03+08:00", /* timestamp */
            FALSE,                        /* expected multipart */
            "尊敬的深*市*泽*业*理*限*司客户：您当前已使用流量总量36.28GB；详细套餐流量使用情况可回复“702”查询。", /* text */
            NULL,    /* expected data */
            0        /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10001",                     /* number */
            "2021-03-25T18:12:59+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "尊敬的深*市*泽*业*理*限*司客户：截止至2月28日，您的手机账户本月可用余额为0.00元，预存费用余额为0.00，赠送费用余额", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10001",                     /* number */
            "2021-03-25T18:13:00+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "为0.00。感谢您的使用！", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
    };
    #if 0
    (void)info;

    for (i=0; hexpdu[i] != NULL; i++)
    {
        GError *error = NULL;
        MMSmsPart *part;
        g_print("index %d=============================", i);
        part = mm_sms_part_3gpp_new_from_pdu(0, hexpdu[i], NULL, &error);
        g_assert_no_error(error);
        show_part_info(part);
        mm_sms_part_free(part);
    }
    #else
    (void)show_part_info;
    for (i=0; hexpdu[i] != NULL; i++)
    {
        info[i].hexpdu=hexpdu[i];
        g_print("index %d=============================", i);
        common_test_part_from_hexpdu(
            info[i].hexpdu,
            info[i].expected_smsc,
            info[i].expected_number,
            info[i].expected_timestamp,
            info[i].expected_multipart,
            info[i].expected_text,
            info[i].expected_data,
            info[i].expected_data_size);
    }
    #endif
}
static void
test_askey_pdu_china_mobile(void)
{
    int i = 0;
    const gchar *hexpdu[] = {

/* need contact */
"6405a10180f600081230528102402389060804002b02015c0a656c7684003100330034"
"003200330038003500330033003000345ba26237ff0c622a81f30032003565e5003100"
"3865f6003200305206ff0c60a85f5367085e3875286d4191cf5df275280030002e0030"
"0030004dff0c53ef75280030ff0c4ee54e0a4fe1606f53ef80fd5b5857285ef665f6ff"
"0c51774f534ee58be653554e3a",
"6405a10180f600081230528102402373060804002b020251c630025982970067e5770b"
"66f4591a8be660c560a853ef767b5f5500200068007400740070003a002f002f006400"
"78002e00310030003000380036002e0063006e002f0071004500650048004200410020"
"ff0c65b94fbf66f45feb6377300230104e2d56fd79fb52a83011",
        NULL,
    };
    struct test_info info[] = {
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10086",                     /* number */
            "2021-03-25T18:20:04+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "尊敬的13423853304客户，截至25日18时20分，您当月常用流量已用0.00M，可用0，以上信息可能存在延时，具体以详单为", /* text */
            NULL,    /* expected data */
            0        /* expected data size */
        },
        {
            NULL,                        /* hex pdu */
            NULL,                        /* smsc */
            "10086",                     /* number */
            "2021-03-25T18:20:04+08:00", /* timestamp */
            TRUE,                        /* expected multipart */
            "准。如需查看更多详情您可登录 http://dx.10086.cn/qEeHBA ，方便更快捷。【中国移动】", /* text */
            NULL,     /* expected data */
            0         /* expected data size */
        },
    };
    #if 0
    (void)info;

    for (i=0; hexpdu[i] != NULL; i++)
    {
        GError *error = NULL;
        MMSmsPart *part;
        g_print("index %d=============================", i);
        part = mm_sms_part_3gpp_new_from_pdu(0, hexpdu[i], NULL, &error);
        g_assert_no_error(error);
        show_part_info(part);
        mm_sms_part_free(part);
    }
    #else
    (void)show_part_info;
    for (i=0; hexpdu[i] != NULL; i++)
    {
        info[i].hexpdu=hexpdu[i];
        g_print("index %d=============================", i);
        common_test_part_from_hexpdu(
            info[i].hexpdu,
            info[i].expected_smsc,
            info[i].expected_number,
            info[i].expected_timestamp,
            info[i].expected_multipart,
            info[i].expected_text,
            info[i].expected_data,
            info[i].expected_data_size);
    }
    #endif
}
#endif

/************************************************************/

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu1", test_pdu1);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu2", test_pdu2);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu3", test_pdu3);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu3-nonzero-pid", test_pdu3_nzpid);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu3-mms", test_pdu3_mms);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu3-natl", test_pdu3_natl);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu3-8bit", test_pdu3_8bit);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-dcsf1", test_pdu_dcsf1);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-dcsf-8bit", test_pdu_dcsf_8bit);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-insufficient-data", test_pdu_insufficient_data);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-udhi", test_pdu_udhi);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-multipart", test_pdu_multipart);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-stored-by-us", test_pdu_stored_by_us);
    g_test_add_func("/MM/SMS/3GPP/PDU-Parser/pdu-not-stored", test_pdu_not_stored);

    g_test_add_func("/MM/SMS/3GPP/Address-Encoder/smsc-intl", test_address_encode_smsc_intl);
    g_test_add_func("/MM/SMS/3GPP/Address-Encoder/smsc-unknown", test_address_encode_smsc_unknown);
    g_test_add_func("/MM/SMS/3GPP/Address-Encoder/intl", test_address_encode_intl);
    g_test_add_func("/MM/SMS/3GPP/Address-Encoder/unknown", test_address_encode_unknown);

    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/UCS2-with-smsc", test_create_pdu_ucs2_with_smsc);
    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/UCS2-no-smsc", test_create_pdu_ucs2_no_smsc);
    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/GSM-with-smsc", test_create_pdu_gsm_with_smsc);
    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/GSM-no-smsc", test_create_pdu_gsm_no_smsc);
    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/GSM-3", test_create_pdu_gsm_3);
    g_test_add_func("/MM/SMS/3GPP/PDU-Creator/GSM-no-validity", test_create_pdu_gsm_no_validity);

    g_test_add_func("/MM/SMS/3GPP/Text-Split/gsm7/short", test_text_split_short_gsm7);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/ucs2/short", test_text_split_short_ucs2);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/utf16/short", test_text_split_short_utf16);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/gsm7/max-single-pdu", test_text_split_max_single_pdu_gsm7);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/ucs2/max-single-pdu", test_text_split_max_single_pdu_ucs2);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/utf16/max-single-pdu", test_text_split_max_single_pdu_utf16);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/gsm7/two-pdu", test_text_split_two_pdu_gsm7);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/ucs2/two-pdu", test_text_split_two_pdu_ucs2);
    g_test_add_func("/MM/SMS/3GPP/Text-Split/utf16/two-pdu", test_text_split_two_pdu_utf16);
    #if CUSTOMER_PDU
    g_test_add_func("/MM/SMS/3GPP/askey/pdu_china_unicom", test_askey_pdu_china_unicom);
    g_test_add_func("/MM/SMS/3GPP/askey/pdu_A1", test_askey_pdu_A1);
    g_test_add_func("/MM/SMS/3GPP/askey/pdu_china_telecom", test_askey_pdu_china_telecom);
    g_test_add_func("/MM/SMS/3GPP/askey/pdu_china_mobile", test_askey_pdu_china_mobile);
    #endif
    // g_test_add_func("/MM/SMS/3GPP/askey/pdu_sms_800", test_askey_pdu4);

    // g_test_add_func ("/MM/SMS/3GPP/askey/pdu_A1",        test_askey_pdu2);

    // if (!mm_log_setup ("DEBUG",
    //                    "./log",
    //                    FALSE,
    //                    TRUE,
    //                    FALSE,
    //                    NULL)) {
    //     g_warning ("failed to set up logging:");
    //     return -1;
    // }
    return g_test_run();
}
