/* -------------------------------------------
   FMOD Studio Script Example:
   Engine Designer Dialog
   -------------------------------------------
 */

studio.menu.addMenuItem({
    name: "FMOD Examples\\Engine Designer",
    keySequence: "Ctrl+Shift+E",
    execute: function() {
    
        var sliderResolution = 10000;
        
        studio.ui.showModelessDialog({
            windowTitle: "Engine Designer",
            windowWidth: 200,
            windowHeight: 400,
            widgetType: studio.ui.widgetType.Layout,
            layout: studio.ui.layoutType.VBoxLayout,
            spacing: 12,
            items: [
                { widgetType: studio.ui.widgetType.Label, widgetId: "m_status", wordWrap: true },
                {
                    widgetType: studio.ui.widgetType.Layout,
                    widgetId: "m_controls",
                    layout: studio.ui.layoutType.HBoxLayout,
                    contentsMargins: { left: 0, top: 0, right: 0, bottom: 0 },
                    spacing: 20,
                    items: [
                        {
                            widgetType: studio.ui.widgetType.Layout,
                            layout: studio.ui.layoutType.VBoxLayout,
                            contentsMargins: { left: 0, top: 0, right: 0, bottom: 0 },
                            alignment: studio.ui.alignment.AlignHCenter,
                            stretchFactor: 1,
                            items: [
                                { widgetType: studio.ui.widgetType.Label, text: "RPM", alignment: studio.ui.alignment.AlignHCenter, },
                                {
                                    row: 1, column: 0,
                                    widgetType: studio.ui.widgetType.Slider,
                                    widgetId: "m_rpmTarget",
                                    range: { minimum: 0, maximum: sliderResolution },
                                    value: 0.413 * sliderResolution,
                                    orientation: studio.ui.orientation.Vertical,
                                    alignment: studio.ui.alignment.AlignHCenter,
                                },
                            ],
                        },
                        {
                            widgetType: studio.ui.widgetType.Layout,
                            layout: studio.ui.layoutType.VBoxLayout,
                            contentsMargins: { left: 0, top: 0, right: 0, bottom: 0 },
                            spacing: 6,
                            sizePolicy: { horizontalPolicy: studio.ui.sizePolicy.Fixed },
                            items: [
                                { widgetType: studio.ui.widgetType.Label, text: "RPM Smooth" },
                                {
                                    widgetType: studio.ui.widgetType.Slider,
                                    widgetId: "m_rpmSmooth",
                                    range: { minimum: 0, maximum: sliderResolution },
                                    value: 0.980 * sliderResolution,
                                    orientation: studio.ui.orientation.Horizontal,
                                },
                                { widgetType: studio.ui.widgetType.Label, text: "Load Smooth" },
                                {
                                    widgetType: studio.ui.widgetType.Slider,
                                    widgetId: "m_loadSmooth",
                                    range: { minimum: 0, maximum: sliderResolution },
                                    value: 0.945 * sliderResolution,
                                    orientation: studio.ui.orientation.Horizontal,
                                },
                                { widgetType: studio.ui.widgetType.Label, text: "Load Scale" },
                                {
                                    widgetType: studio.ui.widgetType.Slider,
                                    widgetId: "m_loadScale",
                                    range: { minimum: 0, maximum: sliderResolution },
                                    value: 0.5921 * sliderResolution,
                                    orientation: studio.ui.orientation.Horizontal,
                                },
                                { widgetType: studio.ui.widgetType.Spacer, stretchFactor: 1 },
                            ],
                        }
                    ],
                },
            ],
            onConstructed: function() {
                this.smoothedRPM = 0;
                this.smoothedLoad = 0;
                this.timerId = this.startTimer(20);
            },
            onTimerEvent: function(timerId) {
                if(timerId !== this.timerId)
                    return;
                
                var event = studio.window.browserCurrent();
                var rpmParameter = null;
                var loadParameter = null;
                
                if(event && event.isOfType("Event")) {    
                    event.getParameterPresets().forEach(function(parameter) {
                        if(new RegExp('RPM', 'gi').test(parameter.presetOwner.name)) {
                            rpmParameter = parameter;
                        }
                        else if(new RegExp('Load', 'gi').test(parameter.presetOwner.name)) {
                            loadParameter = parameter;
                        }
                    });
                }
                
                if(rpmParameter && loadParameter) {
                    this.findWidget("m_status").setText("<font color=\"YellowGreen\">Actively</font> driving <i>{0}</i> and <i>{1}</i> for <i>{2}</i>.".format(rpmParameter.presetOwner.name, loadParameter.presetOwner.name, event.name));
                    this.findWidget("m_controls").setEnabled(true);
                
                    var rpmsmooth = 1 - (this.findWidget("m_rpmSmooth").value() / sliderResolution) * 0.99; // keep this from being 0
                    var loadsmooth = 1 - (this.findWidget("m_loadSmooth").value() / sliderResolution) * 0.99; // keep this from being 0
                    var loadscale = (this.findWidget("m_loadScale").value() / sliderResolution) * 10;
                    var targetRpm = (this.findWidget("m_rpmTarget").value() / sliderResolution);
                    
                    var targetload = (targetRpm - this.smoothedRPM) * loadscale;
                    targetload = (targetload + 1) * 0.5;
                    this.smoothedRPM  += ((targetRpm - this.smoothedRPM) * rpmsmooth);
                    event.setCursorPosition(rpmParameter, rpmParameter.minimum + (this.smoothedRPM * (rpmParameter.maximum - rpmParameter.minimum)));
                    
                    this.smoothedLoad += ((targetload - this.smoothedLoad)) * loadsmooth;
                    event.setCursorPosition(loadParameter, loadParameter.minimum + (this.smoothedLoad * (loadParameter.maximum - loadParameter.minimum)));
                }
                else {
                    this.findWidget("m_status").setText("Please <font color=\"Red\">select an event</font> with an <i>RPM</i> and <i>Load</i> parameter.");
                    this.findWidget("m_controls").setEnabled(false);
                }
            }
        });
    },
});
