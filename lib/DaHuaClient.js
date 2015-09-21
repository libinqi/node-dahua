var dahua = require('../bin/dahua.node');
var client=new dahua.DaHuaClient();

exports.init = function (deviceIP, devicePort, userName, password, disconnected) {
	var result=client.Init_Device(function(data){
		 if(disconnected)
			disconnected(data);
	});
	if(result)
	{
		var loginId = client.Login_Device(deviceIP,devicePort,userName,password);
		return loginId;
	}
	return 0;
};

exports.startRealPlay = function (loginId,width,height,callback) {
	var realHandle=client.StartRealPlay(loginId,width,height,function(size,buf){
		 if(callback)
			callback(size,buf);
		});

  return realHandle;
};

exports.stopRealPlay = function (realHandleId) {
	var result=client.StopRealPlay(realHandleId);
  return result;
};

exports.subscribeCarNumber = function (loginId,callback) {
	var realHandle=client.SubscribeCarNumber(loginId,function(carNumber){
	 if(callback)
		callback(carNumber);
	});

  return realHandle;
};
