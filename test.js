var client = require('./lib/DaHuaClient');

var loginId = client.init("61.234.234.164",37777,"admin","admin");
if(loginId&&loginId!=0)
{
	console.log('设备注册Id:', loginId);

	var realHandle=client.startRealPlay(loginId,820,410,function(size,buf){
	 console.log(buf);
	  // decoder.H264Decoder(size,buf,function(data){
		 // console.log(data);
	  // });
	});
	if(realHandle&&realHandle!=0)
	{
		console.log('开启实时监控:', realHandle);
	}
	else{
		console.log('开启实时监控失败！');
	}
}
else{
	console.log('设备注册失败！');
}


process.on('uncaughtException', function (err) {
    console.log(err);
});

process.stdin.resume();
