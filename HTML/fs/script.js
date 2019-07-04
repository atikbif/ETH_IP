var xhr;
var dyn_code = "";  
  function onload(){
	dyn_code = self.location.href ;
	var i = dyn_code.indexOf("page");
	if(i>=0) {
		dyn_code = dyn_code.substring(i+4,i+12);
	}else dyn_code="error";
	xhr = new(XMLHttpRequest);
	read_data();
	setInterval(function(){ read_can();},5000);
	
  }
  function read_can(){
	if(document.getElementById("tab_3").checked) {
		var rxhr;
		rxhr = new(XMLHttpRequest);
		rxhr.open("GET", "can.html?code="+dyn_code+"&r=" + Math.random(), true); 
		rxhr.responseType = "text";
		rxhr.onload = function (oEvent) {
			if(rxhr.status == 200) {
				var full_answer = rxhr.response;
				document.getElementById('cur_time').innerHTML = full_answer.substring(0,18);
				document.getElementById('can_data').innerHTML = full_answer.substring(18);
			}else {
				Toast.add({
				  text: 'Отказано в доступе',
				  color: '#ff9f9f',
				  autohide: true
				});
			}
		}
		rxhr.send(null);
	}
  }
  function read_data(){
	var rxhr;
	rxhr = new(XMLHttpRequest);
	rxhr.open("GET", "read.html?code="+dyn_code+"&r=" + Math.random(), true); 
	rxhr.responseType = "text";
	rxhr.onload = function (oEvent) {
	
		if(rxhr.status == 200) {
			Toast.add({
			  text: 'Настройки прочитаны',
			  color: '#9fff9f',
			  autohide: true
			});
		
			document.getElementById('version_high').innerHTML = rxhr.response[0];
			document.getElementById('version_low').innerHTML = rxhr.response[1];
			document.getElementById('node0').value = rxhr.response[2];
			document.getElementById('node1').value = rxhr.response[3];
			document.getElementById('node2').value = rxhr.response[4];
			document.getElementById('node3').value = rxhr.response[5];
			document.getElementById('node4').value = rxhr.response[6];
			document.getElementById('node5').value = rxhr.response[7];
			document.getElementById('node6').value = rxhr.response[8];
			var s = rxhr.response[9]+rxhr.response[10]+rxhr.response[11];if(!isNaN(s)) document.getElementById('ip1').value = s;else document.getElementById('ip1').value = "000";
			s = rxhr.response[12]+rxhr.response[13]+rxhr.response[14];if(!isNaN(s)) document.getElementById('ip2').value = s;else document.getElementById('ip2').value = "000";
			s = rxhr.response[15]+rxhr.response[16]+rxhr.response[17];if(!isNaN(s)) document.getElementById('ip3').value = s;else document.getElementById('ip3').value = "000";
			s = rxhr.response[18]+rxhr.response[19]+rxhr.response[20];if(!isNaN(s)) document.getElementById('ip4').value = s;else document.getElementById('ip4').value = "000";
			s = rxhr.response[21]+rxhr.response[22]+rxhr.response[23];if(!isNaN(s)) document.getElementById('mask1').value = s;else document.getElementById('mask1').value = "000";
			s = rxhr.response[24]+rxhr.response[25]+rxhr.response[26];if(!isNaN(s)) document.getElementById('mask2').value = s;else document.getElementById('mask2').value = "000";
			s = rxhr.response[27]+rxhr.response[28]+rxhr.response[29];if(!isNaN(s)) document.getElementById('mask3').value = s;else document.getElementById('mask3').value = "000";
			s = rxhr.response[30]+rxhr.response[31]+rxhr.response[32];if(!isNaN(s)) document.getElementById('mask4').value = s;else document.getElementById('mask4').value = "000";
			s = rxhr.response[33]+rxhr.response[34]+rxhr.response[35];if(!isNaN(s)) document.getElementById('gate1').value = s;else document.getElementById('gate1').value = "000";
			s = rxhr.response[36]+rxhr.response[37]+rxhr.response[38];if(!isNaN(s)) document.getElementById('gate2').value = s;else document.getElementById('gate2').value = "000";
			s = rxhr.response[39]+rxhr.response[40]+rxhr.response[41];if(!isNaN(s)) document.getElementById('gate3').value = s;else document.getElementById('gate3').value = "000";
			s = rxhr.response[42]+rxhr.response[43]+rxhr.response[44];if(!isNaN(s)) document.getElementById('gate4').value = s;else document.getElementById('gate4').value = "000";
			s = rxhr.response[45]+rxhr.response[46]+rxhr.response[47]+rxhr.response[48]+rxhr.response[49];if(!isNaN(s)) document.getElementById('id').value = s;else document.getElementById('id').value = "00000";
		}else {
			Toast.add({
			  text: 'Отказано в доступе',
			  color: '#ff9f9f',
			  autohide: true
			});
		}
	}
	rxhr.send(null);
  }
  function write_data(){
	var count = 0;
	var pc_name = "node";	
	var	res = "";
	while (count < 7) {
		var node = document.getElementById(pc_name+(count));
		var value = node.options[node.selectedIndex].value;
		res+=value;
		count++;
	}
	
	var ip_check = true;
	var mask_check = true;
	var gate_check = true;
	var id_check = true;
	
	var ip1=document.getElementById('ip1').value; while(ip1.length<3) ip1='0'+ip1;if(isNaN(ip1)) ip_check = false;else res+=ip1;
	var ip2=document.getElementById('ip2').value; while(ip2.length<3) ip2='0'+ip2;if(isNaN(ip2)) ip_check = false;else res+=ip2;
	var ip3=document.getElementById('ip3').value; while(ip3.length<3) ip3='0'+ip3;if(isNaN(ip3)) ip_check = false;else res+=ip3;
	var ip4=document.getElementById('ip4').value; while(ip4.length<3) ip4='0'+ip4;if(isNaN(ip4)) ip_check = false;else res+=ip4;
	
	var mask1=document.getElementById('mask1').value; while(mask1.length<3) mask1='0'+mask1;if(isNaN(mask1)) mask_check = false;else res+=mask1;
	var mask2=document.getElementById('mask2').value; while(mask2.length<3) mask2='0'+mask2;if(isNaN(mask2)) mask_check = false;else res+=mask2;
	var mask3=document.getElementById('mask3').value; while(mask3.length<3) mask3='0'+mask3;if(isNaN(mask3)) mask_check = false;else res+=mask3;
	var mask4=document.getElementById('mask4').value; while(mask4.length<3) mask4='0'+mask4;if(isNaN(mask4)) mask_check = false;else res+=mask4;
	
	var gate1=document.getElementById('gate1').value; while(gate1.length<3) gate1='0'+gate1;if(isNaN(gate1)) gate_check = false;else res+=gate1;
	var gate2=document.getElementById('gate2').value; while(gate2.length<3) gate2='0'+gate2;if(isNaN(gate2)) gate_check = false;else res+=gate2;
	var gate3=document.getElementById('gate3').value; while(gate3.length<3) gate3='0'+gate3;if(isNaN(gate3)) gate_check = false;else res+=gate3;
	var gate4=document.getElementById('gate4').value; while(gate4.length<3) gate4='0'+gate4;if(isNaN(gate4)) gate_check = false;else res+=gate4;
	
	var id1=document.getElementById('id').value; while(id1.length<5) id1='0'+id1;if(isNaN(id1)) id_check = false;else res+=id1;
	
	if(ip_check && mask_check && gate_check && id_check) {
		xhr.open("GET", "write.html?code="+dyn_code+"&node="+res, true);
		xhr.responseType = "text";
		xhr.onload = function (oEvent) {
			if(xhr.status == 200) {
				Toast.add({
				  text: 'Запись настроек',
				  color: '#9fff9f',
				  autohide: true
				});
			}else {
				Toast.add({
				  text: 'Отказано в доступе',
				  color: '#ff9f9f',
				  autohide: true
				});
			}
		}
		xhr.send(null);
		
	}else {
		if(ip_check==false) {
			Toast.add({
			  text: 'Некорректный IP адрес',
			  color: '#ff9f9f',
			  autohide: false
			});
		}
		if(mask_check==false) {
			Toast.add({
			  text: 'Некорректная IP маска',
			  color: '#ff9f9f',
			  autohide: false
			});
		}
		if(gate_check==false) {
			Toast.add({
			  text: 'Некорректный IP шлюз',
			  color: '#ff9f9f',
			  autohide: false
			});
		}
		if(id_check==false) {
			Toast.add({
			  text: 'Некорректный идентификатор',
			  color: '#ff9f9f',
			  autohide: false
			});
		}
	}
  }
  function reset_device(){
	xhr.open("GET", "reset.html?code="+dyn_code, true);
	xhr.responseType = "text";
	xhr.onload = function (oEvent) {
		if(xhr.status == 200) {
			var ip1=document.getElementById('ip1').value; 
			var ip2=document.getElementById('ip2').value; 
			var ip3=document.getElementById('ip3').value; 
			var ip4=document.getElementById('ip4').value;
			ip1 = ip1.replace(/^0+/, "");
			ip2 = ip2.replace(/^0+/, "");
			ip3 = ip3.replace(/^0+/, "");
			ip4 = ip4.replace(/^0+/, "");
			if((!isNaN(ip1)) && (!isNaN(ip2)) && (!isNaN(ip3)) && (!isNaN(ip4))) {
			
			
				Toast.add({
				  text: 'Обновление страницы' + " " + ip1 + "." + ip2 + "." + ip3 + "." + ip4,
				  color: '#9fff9f',
				  autohide: true
				});
				
				setTimeout(function(){ var page_addr = "http://" + ip1 + "." + ip2 + "." + ip3 + "." + ip4;	location.href=page_addr;},2000);
			}
		}else {
			Toast.add({
			  text: 'Отказано в доступе',
			  color: '#ff9f9f',
			  autohide: true
			});
		}
	}
	
	xhr.send(null);
	
	
  }  
  
  function synchro(){
	var pageName = "synchro.html?code="+dyn_code+"&date=";
	var currentdate = new Date(); 
	var date = currentdate.getDate();
	if(date<10) pageName+="0"+date;else pageName+=date;
	var month = currentdate.getMonth()+1;
	if(month<10) pageName+="0"+month;else pageName+=month;
	var year = currentdate.getFullYear()-2000;
	if(year<10) pageName+="0"+year;else pageName+=year;
	var hours = currentdate.getHours();
	if(hours<10) pageName+="0"+hours;else pageName+=hours;
	var min = currentdate.getMinutes();
	if(min<10) pageName+="0"+min;else pageName+=min;
	var sec = currentdate.getSeconds();
	if(sec<10) pageName+="0"+sec;else pageName+=sec;;
	xhr.open("GET", pageName, true);
	xhr.responseType = "text";
	xhr.onload = function (oEvent) {
		if(xhr.status == 200) {
			Toast.add({
			  text: 'Время записано',
			  color: '#9fff9f',
			  autohide: true
			});
		}else {
			Toast.add({
			  text: 'Отказано в доступе',
			  color: '#ff9f9f',
			  autohide: true
			});
		}			
	}
	xhr.send(null);
  }  
  
  function new_password(){
		var pageName = "http://"+self.location.hostname + "/new_password.html";
		location.replace(pageName);
  }
  function home_page(){
	var pageName = "http://"+self.location.hostname;
		location.replace(pageName);
  }
  
  var Toast = function (element, config) {
  // приватные переменные класса Toast
  var
	_this = this,
	_element = element,
	_config = {
	  autohide: true,
	  delay: 5000
	};
  // установление _config
  for (var prop in config) {
	_config[prop] = config[prop];
  }
  // get-свойство element
  Object.defineProperty(this, 'element', {
	get: function () {
	  return _element;
	}
  });
  // get-свойство config
  Object.defineProperty(this, 'config', {
	get: function () {
	  return _config;
	}
  });
  // обработки события click (скрытие сообщения при нажатии на кнопку "Закрыть")
  _element.addEventListener('click', function (e) {
	if (e.target.classList.contains('toast__close')) {
	  _this.hide();
	}
  });
}
// методы show и hide, описанные в прототипе объекта Toast
Toast.prototype = {
  show: function () {
	var _this = this;
	this.element.classList.add('toast_show');
	if (this.config.autohide) {
	  setTimeout(function () {
		_this.hide();
	  }, this.config.delay)
	}
  },
  hide: function () {
	this.element.classList.remove('toast_show');
  }
};
// статическая функция для Toast (используется для создания сообщения)
Toast.create = function (text, color) {
  var
	fragment = document.createDocumentFragment(),
	toast = document.createElement('div'),
	toastClose = document.createElement('button');
  toast.classList.add('toast');
  toast.style.backgroundColor = 'rgba(' + parseInt(color.substr(1, 2), 16) + ',' + parseInt(color.substr(3, 2), 16) + ',' + parseInt(color.substr(5, 2), 16) + ',0.5)';
  toast.textContent = text;
  toastClose.classList.add('toast__close');
  toastClose.setAttribute('type', 'button');
  toastClose.textContent = '×';
  toast.appendChild(toastClose);
  fragment.appendChild(toast);
  return fragment;
};
// статическая функция для Toast (используется для добавления сообщения на страницу)
Toast.add = function (params) {
  var config = {
	header: 'Название заголовка',
	text: 'Текст сообщения...',
	color: '#ffffff',
	autohide: true,
	delay: 5000
  };
  if (params !== undefined) {
	for (var item in params) {
	  config[item] = params[item];
	}
  }
  if (!document.querySelector('.toasts')) {
	var container = document.createElement('div');
	container.classList.add('toasts');
	container.style.cssText = 'position: fixed; top: 15px; right: 15px; width: 250px;';
	document.body.appendChild(container);
  }
  document.querySelector('.toasts').appendChild(Toast.create(config.text, config.color));
  var toasts = document.querySelectorAll('.toast');
  var toast = new Toast(toasts[toasts.length - 1], { autohide: config.autohide, delay: config.delay });
  toast.show();
  return toast;
}