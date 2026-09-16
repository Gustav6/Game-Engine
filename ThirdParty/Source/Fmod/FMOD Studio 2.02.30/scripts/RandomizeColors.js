/* -------------------------------------------
   FMOD Studio Script Example:
   Randomize Multitrack Colors
   -------------------------------------------
 */

studio.menu.addMenuItem({ name: "FMOD Examples\\Randomize Multitrack Colors",
	isEnabled: function() { var event = studio.window.browserCurrent(); return event && event.isOfExactType("Event"); },
	keySequence: "Alt+R",
	execute: function() {
		var colors = ["Blue", "Cyan", "Green", "Magenta", "Red", "Yellow"];
		var event = studio.window.browserCurrent();
		event.groupTracks.forEach(function(track) {
			var randomColor = colors[Math.floor(Math.random() * colors.length)];
			var randomLightness = Math.floor(Math.random() * 3);
			var colorName = randomColor + (randomLightness ? " Light " + randomLightness : "");
			track.mixerGroup.color = colorName;
			track.modules.forEach(function(module) {
				module.color = colorName;
			});
		});
	}
});