var _translation = null;

function loadTranslation(lang, done) {
    $.getJSON("https://boot.ddnss.de/dawn/dawn_" + lang + ".json").done(
                function (resp) {
                    _translation = resp;
                    $("span.tr").each(function(){
                        this.innerText = tr(this.innerText);
                    });
                    $("input.tr").each(function() {
                        this.value = tr(this.value);
                    });
                }).always(done);
}

function tr(str) {
  if (null === _translation)
      return str;
  if (str in _translation)
      return _translation[str];
  return str;
}

function formatDayOfWeek(num) {
  if (65 == num)  return tr("weekend");
  if (62 == num)  return tr("work days");
  if (127 == num) return tr("always");
  if (0 == num) return tr("never");
  var res = [];
  if (2  & num) res.push(tr("monday"));
  if (4  & num) res.push(tr("tuesday"));
  if (8  & num) res.push(tr("wednesday"));
  if (16 & num) res.push(tr("thursday"));
  if (32 & num) res.push(tr("friday"));
  if (64 & num) res.push(tr("saturday"));
  if (1  & num) res.push(tr("sunday"));
  return res.join(", ");
}

function formatAlarm(idx, alarm) {
  return '<li id="#alarm' + idx + '"><a href="#" id="alink' + idx + '">'
    + formatDayOfWeek(alarm.dayofweek) + tr(' at ') + alarm.time + '</a></li>';
}

function saveAlarm(event) {
  var dayofweek = 0;
  if ("on" == $("#sun").val()) dayofweek |=  1;
  if ("on" == $("#mon").val()) dayofweek |=  2;
  if ("on" == $("#tue").val()) dayofweek |=  4;
  if ("on" == $("#wed").val()) dayofweek |=  8;
  if ("on" == $("#thu").val()) dayofweek |= 16;
  if ("on" == $("#fri").val()) dayofweek |= 32;
  if ("on" == $("#sat").val()) dayofweek |= 64;
  var time = $("#start").val();
  $.getJSON("https://boot.ddnss.de/dawn/api", {q:'setalarm', idx:event.data.idx,
            dow:dayofweek, time:time}).done(function (resp) {
    $.mobile.changePage("#main");
    updateAlarmList();
  });
}

function showSetAlarm(event) {
 if (1 & event.data.dayofweek) $("#sun").val("on")
 else $("#sun").val("off")
 if (2 & event.data.dayofweek) $("#mon").val("on")
 else $("#mon").val("off")
 if (4 & event.data.dayofweek) $("#tue").val("on")
 else $("#tue").val("off")
 if (8 & event.data.dayofweek) $("#wed").val("on")
 else $("#wed").val("off")
 if (16 & event.data.dayofweek) $("#thu").val("on")
 else $("#thu").val("off")
 if (32 & event.data.dayofweek) $("#fri").val("on")
 else $("#fri").val("off")
 if (64 & event.data.dayofweek) $("#sat").val("on")
 else $("#sat").val("off")
 $("#start").val(event.data.time);

 $.mobile.changePage("#setalarm", "pop", true, true);

 $("#sun").slider("refresh");
 $("#mon").slider("refresh");
 $("#tue").slider("refresh");
 $("#wed").slider("refresh");
 $("#thu").slider("refresh");
 $("#fri").slider("refresh");
 $("#sat").slider("refresh");

 $("#savealarm").click({idx:event.data.idx}, saveAlarm);
}

function updateAlarmList() {
  $("#alarm").empty();
  // JSON request to get list of alarms
  $.getJSON("https://boot.ddnss.de/dawn/api", {q:'list'}).done(function(resp) {
    for (i=0; i<resp.length; i++) {
      $("#alarm").append(formatAlarm(i, resp[i]));
      $("#alink"+i).click({idx:i, dayofweek:resp[i].dayofweek, time:resp[i].time}, showSetAlarm);
    }
    $("#alarm").listview("refresh");
  });
}

$( document ).ready(function() {
    var lang = navigator.language || navigator.userLanguage;
    loadTranslation(lang, function() {
        // Update list of alarm settings
        updateAlarmList();

        // JSON request to get the current value
        $.getJSON("https://boot.ddnss.de/dawn/api", {q:'value'}).done(function(resp) {
            $("#value").val(resp.value).slider("refresh")
        });

        $("#value").on('change', function () {
            $.getJSON("https://boot.ddnss.de/dawn/api", {q:'setvalue', value: $("#value").val()});
        });

        setInterval(function() {
            $.getJSON("https://boot.ddnss.de/dawn/api", {q:'time'}).done(function(resp)  {
                $("#time").empty();
                $("#time").append(resp.time);
            });

            $.getJSON("https://boot.ddnss.de/dawn/api", {q:'temp'}).done(function(resp)  {
                $("#temp").empty();
              $("#temp").append(Math.round(resp.temp*10)/10 + "&deg;C");
            });
        }, 10000);
    });
});
