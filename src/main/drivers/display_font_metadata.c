#include "drivers/display_font_metadata.h"
#include "drivers/osd.h"

bool displayFontMetadataUpdateFromCharacter(displayFontMetadata_t *metadata, const osdCharacter_t *chr)
{
    if (chr && FONT_CHR_IS_METADATA(chr)) {
        metadata->version = chr->data[5];
        return true;
    }
    return false;
}
