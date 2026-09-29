#ifndef HTTP_HEADER_FIELDNAME_CODEGEN
#define HTTP_HEADER_FIELDNAME_CODEGEN

#ifndef INVALID_FIELD_NAME_CHARS
#define INVALID_FIELD_NAME_CHARS "()<>@,;:\\\"/[]?={} \t"

#endif

static char invalid_chars[] = INVALID_FIELD_NAME_CHARS;

#define INV_CHAR_LEN (sizeof(INVALID_FIELD_NAME_CHARS) - 1)

//TODO: should return pointer to string
void generate_masks()
{
    int c;

    uint64_t mask1 = 0, mask2 = 0, mask3 = 0;

    for (c = 0; c < 127; c++)
    {
        if (c < 33)
        {
            continue;
        }

        if (c != ':')
        {
            size_t j;
            for (j = 0; j < INV_CHAR_LEN; j++)
            {
                if (c == invalid_chars[j])
                {
                    break;
                }
            }

            if (j < INV_CHAR_LEN)
            {
                continue;
            }
        }

        if (c < 64)
        {
            mask1 |= (0x1ul) << c;
        }
        else
        {
            mask2 |= (0x1ul) << (c - 64);
            if (c >= 'A' && c <= 'Z')
            {
                // to allow same shift to right and & with 32
                mask3 |= (32) << (c - 64);
            }
        }
    }

    printf("/*!< HEADER_FIELD_NAME_VALID_CHARS1_BITS:bits 0-64*/\n#define HEADER_FIELD_NAME_VALID_CHARS1_BITS (0x%lxul)\n", mask1);
    printf("/*!< HEADER_FIELD_NAME_VALID_CHARS2_BITS:bits 65-128*/\n#define HEADER_FIELD_NAME_VALID_CHARS2_BITS (0x%lxul)\n", mask2);
    printf("/*!< HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS:for uppercase*/\n#define HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS (0x%lxul)\n", mask3);
}

#endif