var NoStatus = "Current Status: N/A";

function ProcessData(Data,Index,SetControls){
  if(Data != null && Index != null){
    var data_array = Data.split('_');
    if(data_array.length == 4){
      if(data_array[0] == "O") data_array[0] = "ON";
      else data_array[0] = "OFF";
      if(data_array[2] == "O") data_array[2] = "ON";
      else data_array[2] = "OFF";
      if (Index == "LED"){
        if(SetControls == true){
          if(data_array[0] == "ON") $("#swled").prop("checked",true);
          else $("#swled").prop("checked",false);
          $("#leddimmerlevel").val(data_array[1]);
          $("#ledlevel").text($("#leddimmerlevel").val() + "%");
        }
        return "Current Status: LED " + data_array[0] + " , LED Intensity : " + data_array[1] + "%";
      }
      else if(Index == "FAN"){
        if(SetControls == true){
          if(data_array[2] == "ON") $("#swfan").prop("checked",true);
          else $("#swfan").prop("checked",false);
          $("#fanspeedlevel").val(data_array[3]);
          $("#fanlevel").text($("#fanspeedlevel").val() + "%");
        }
        return "Current Status: FAN " + data_array[2] + " , FAN Speed : " + data_array[3] + "%";
      }
      else return NoStatus;
    }
    else return NoStatus;
  }
  else return NoStatus;
}

function DisplayError(){
  alert('Unable to connect to Wifi Module');
}

function PollData(){
  $.ajax({
    url: "/REFRESH",
    method: "POST",
    success: function(result){
      $("#ledcurrstatus").text(ProcessData(result,"LED",true));
      $("#fancurrstatus").text(ProcessData(result,"FAN",true));
    },
    error: function(result){
      DisplayError();
      $("#ledcurrstatus").text(NoStatus);
      $("#fancurrstatus").text(NoStatus);
    }
  });
}
$(document).ready(function(){
  $("#ledlevel").text($("#leddimmerlevel").val() + "%");
  $("#leddimmerlevel").on("input",function(){
    $("#ledlevel").text($("#leddimmerlevel").val() + "%");
    $.ajax({
      url: "/LEDDIM",
      method: "POST",
      data: {"LEDDIMLevel" : $("#leddimmerlevel").val() },
      success: function(result){
        $("#ledcurrstatus").text(ProcessData(result,"LED"));
      },
      error: function(result){
        DisplayError();
      }
    });
  });
  $("#fanspeedlevel").on("input",function(){
    $("#fanlevel").text($("#fanspeedlevel").val() + "%");
    $.ajax({
      url: "/FANSPEED",
      method: "POST",
      data: {"FANSppedLevel" : $("#fanspeedlevel").val() },
      success: function(result){
        $("#fancurrstatus").text(ProcessData(result,"FAN"));
      },
      error: function(result){
        DisplayError();
      }
    });
  });
  $("#fanlevel").text($("#fanspeedlevel").val() + "%");
  $("#swled").change(function(){
    if($("#swled").prop("checked")){
      $.ajax({
        url: "/LEDON",
        method: "POST",
        success: function(result){
          $("#ledcurrstatus").text(ProcessData(result,"LED"));
        },
        error: function(result){
          DisplayError();
        }
      });
    }
    else {
      $.ajax({
        url: "/LEDOFF",
        method: "POST",
        success: function(result){
          $("#ledcurrstatus").text(ProcessData(result,"LED"));
        },
        error: function(result){
          DisplayError();
        }
      });
    }
  });
  $("#swfan").change(function(){
    if($("#swfan").prop("checked")){
      $.ajax({
        url: "/FANON",
        method: "POST",
        success: function(result){
          $("#fancurrstatus").text(ProcessData(result,"FAN"));
        },
        error: function(result){
          DisplayError();
        }
      });
    }
    else {
      $.ajax({
        url: "/FANOFF",
        method: "POST",
        success: function(result){
          $("#fancurrstatus").text(ProcessData(result,"FAN"));
        },
        error: function(result){
          DisplayError();
        }
      });
    }
  });
  PollData();
  setInterval(PollData, 5000);
});
