#include "senseHat.h"
#include <unistd.h>

char message[8][69] =
    {
        "  ##### ###### #      # ###    #      # #     # ###   ######  ##### ",
        " #      #      ##     # #  #   ##     # #     # #  #  #      #      ",
        " #      #      # #    # #   #  # #    # #     # #   # #      #      ",
        "  ####  ####   #  #   # #   #  #  #   # #     # #   # ####    ####  ",
        "      # #      #   #  # #   #  #   #  # #     # #   # #           # ",
        "      # #      #    # # #   #  #    # # #     # #   # #           # ",
        "      # #      #     ## #  #   #     ## #     # #  #  #           # ",
        " #####  ###### #      # ###    #      #  #####  ###   ###### #####  "};

int v = 0;

int main(int argc, char *argv[])
{

    for (int l = 0; l < 61; l++)
    {
        SenseHat *s = SenseHat_creer();
        SenseHat_initialiserDalle(s);

        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 69; x++)
            {
                if (message[y][x] == '#')
                {
                    if (x < 9)
                    {
                        SenseHat_modifierEtatLED(s, x + 1, y + 1, l, 50, 50);
                    }
                    message[y][x]     = ' ';
                    message[y][x - 1] = '#';
                }
            }
        }

        SenseHat_detruire(s);
        
        // sleep(1);
    }

    return 0;
}