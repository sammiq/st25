/* The scripts of every room. */

#include "rooms.h"

extern const RoomScript demon0_scripts[];
extern const RoomScript demon1_scripts[];
extern const RoomScript demon2_scripts[];
extern const RoomScript demon3_scripts[];
extern const RoomScript demon4_scripts[];
extern const RoomScript demon5_scripts[];
extern const RoomScript demon6_scripts[];
extern const RoomScript tug0_scripts[];
extern const RoomScript tug1_scripts[];
extern const RoomScript tug2_scripts[];
extern const RoomScript tug3_scripts[];
extern const RoomScript love0_scripts[];
extern const RoomScript love1_scripts[];
extern const RoomScript love2_scripts[];
extern const RoomScript love3_scripts[];
extern const RoomScript love4_scripts[];
extern const RoomScript love5_scripts[];
extern const RoomScript mudd0_scripts[];
extern const RoomScript mudd1_scripts[];
extern const RoomScript mudd2_scripts[];
extern const RoomScript mudd3_scripts[];
extern const RoomScript mudd4_scripts[];
extern const RoomScript mudd5_scripts[];
extern const RoomScript feather0_scripts[];
extern const RoomScript feather1_scripts[];
extern const RoomScript feather2_scripts[];
extern const RoomScript feather3_scripts[];
extern const RoomScript feather4_scripts[];
extern const RoomScript feather5_scripts[];
extern const RoomScript feather6_scripts[];
extern const RoomScript feather7_scripts[];
extern const RoomScript trial0_scripts[];
extern const RoomScript trial1_scripts[];
extern const RoomScript trial2_scripts[];
extern const RoomScript trial3_scripts[];
extern const RoomScript trial4_scripts[];
extern const RoomScript trial5_scripts[];
extern const RoomScript trial9_scripts[];
extern const RoomScript sins0_scripts[];
extern const RoomScript sins1_scripts[];
extern const RoomScript sins2_scripts[];
extern const RoomScript sins3_scripts[];
extern const RoomScript sins4_scripts[];
extern const RoomScript sins5_scripts[];
extern const RoomScript veng0_scripts[];
extern const RoomScript veng1_scripts[];
extern const RoomScript veng2_scripts[];
extern const RoomScript veng3_scripts[];
extern const RoomScript veng4_scripts[];
extern const RoomScript veng5_scripts[];
extern const RoomScript veng6_scripts[];
extern const RoomScript veng7_scripts[];
extern const RoomScript veng8_scripts[];

static const struct {
    const char *name;
    const RoomScript *scripts;
} rooms[] = {
    { "demon0", demon0_scripts },
    { "demon1", demon1_scripts },
    { "demon2", demon2_scripts },
    { "demon3", demon3_scripts },
    { "demon4", demon4_scripts },
    { "demon5", demon5_scripts },
    { "demon6", demon6_scripts },
    { "tug0", tug0_scripts },
    { "tug1", tug1_scripts },
    { "tug2", tug2_scripts },
    { "tug3", tug3_scripts },
    { "love0", love0_scripts },
    { "love1", love1_scripts },
    { "love2", love2_scripts },
    { "love3", love3_scripts },
    { "love4", love4_scripts },
    { "love5", love5_scripts },
    { "mudd0", mudd0_scripts },
    { "mudd1", mudd1_scripts },
    { "mudd2", mudd2_scripts },
    { "mudd3", mudd3_scripts },
    { "mudd4", mudd4_scripts },
    { "mudd5", mudd5_scripts },
    { "feather0", feather0_scripts },
    { "feather1", feather1_scripts },
    { "feather2", feather2_scripts },
    { "feather3", feather3_scripts },
    { "feather4", feather4_scripts },
    { "feather5", feather5_scripts },
    { "feather6", feather6_scripts },
    { "feather7", feather7_scripts },
    { "trial0", trial0_scripts },
    { "trial1", trial1_scripts },
    { "trial2", trial2_scripts },
    { "trial3", trial3_scripts },
    { "trial4", trial4_scripts },
    { "trial5", trial5_scripts },
    { "trial9", trial9_scripts },
    { "sins0", sins0_scripts },
    { "sins1", sins1_scripts },
    { "sins2", sins2_scripts },
    { "sins3", sins3_scripts },
    { "sins4", sins4_scripts },
    { "sins5", sins5_scripts },
    { "veng0", veng0_scripts },
    { "veng1", veng1_scripts },
    { "veng2", veng2_scripts },
    { "veng3", veng3_scripts },
    { "veng4", veng4_scripts },
    { "veng5", veng5_scripts },
    { "veng6", veng6_scripts },
    { "veng7", veng7_scripts },
    { "veng8", veng8_scripts },
    { 0, 0 }
};

const RoomScript *FindRoomScripts(const char *room_name)
{
    int i;

    for (i = 0; rooms[i].name; i++)
        if (strcasecmp(rooms[i].name, room_name) == 0)
            return rooms[i].scripts;
    return 0;
}
