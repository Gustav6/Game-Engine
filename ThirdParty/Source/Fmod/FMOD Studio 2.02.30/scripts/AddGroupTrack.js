/* -------------------------------------------
   FMOD Studio Script Example:
   Add Group Track
   -------------------------------------------
 */

studio.menu.addMenuItem({ name: "FMOD Examples\\Add Group Track",
    isEnabled: function() { var event = studio.window.browserCurrent(); return event && event.isOfExactType("Event"); },
    execute: function() {
        var trackName = studio.system.getText("Name of new group track:", "New Track");
        if(trackName) {
            var event = studio.window.browserCurrent();
            var track = studio.project.create("GroupTrack");
            track.mixerGroup.output = event.mixer.masterBus;
            track.mixerGroup.name = trackName;
            event.relationships.groupTracks.add(track);
        }
    },
});
