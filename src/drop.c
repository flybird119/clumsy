// dropping packet module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short dropEnabled = 0,
    dropInbound = 0, dropOutbound = 0,
    chance = 100; // [0-1000]


static Ihandle* dropSetupUI() {
    Ihandle *dropControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Drop Inbound", NULL),
        outboundCheckbox = IupToggle("Drop Outbound", NULL),
        IupLabel("Chance(%):"),
        chanceInput = IupText(NULL),
        NULL
    );

    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(chanceInput, "VALUE", "10.0");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(chanceInput, SYNCED_VALUE, (char*)&chance);
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&dropInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&dropOutbound);

    return dropControlsBox;
}

static void dropStartUp() {
    LOG("drop enabled");
}

static void dropCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("drop disabled");
}

static void dropProcess(PacketNode *head, PacketNode* tail) {
    while (head->next != tail) {
        PacketNode *pac = head->next;
        // due to the threading issue the chance here may change between the first
        // and the second read. I think I'm aware of this but it's fine here i suppose.
        // chance in range of [0, 1000]
        if (((dropInbound && IS_INBOUND(pac->addr.Direction)) 
             || (dropOutbound && IS_OUTBOUND(pac->addr.Direction))
            ) && calcChance(chance)) {
            LOG("droped with chance %.1f% , direction %s",
                chance/10.0, BOUND_TEXT(pac->addr.Direction));
            freeNode(popNode(pac));
        } else {
            head = head->next;
        }
    }
}


Module dropModule = {
    "Drop",
    (short*)&dropEnabled,
    dropSetupUI,
    dropStartUp,
    dropCloseDown,
    dropProcess
};
