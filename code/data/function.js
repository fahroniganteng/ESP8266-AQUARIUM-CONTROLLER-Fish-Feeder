var stopMsg = false;
var homeTimeout;
var ntpPageTimeout;
var errBoard = 'Cannot connect board...';
var ronn	= {
	init	: function(){
		$('[data-toggle="offcanvas"]').on('click', function () {
			$('.offcanvas-collapse').toggleClass('open');
		});
		$('.offcanvas-collapse').on('click','.nav-item',function() {
			var $self		= $(this);
			var container	= $self.data('container');
			$self.prevAll().removeClass('active');
			$self.nextAll().removeClass('active');
			$self.addClass('active');
			$('.offcanvas-collapse').removeClass('open');
			loading('show',function(){
				$('main').hide();
				$('#'+container).show();
				var target	= $self.data('container');
				clearTimeout(homeTimeout);
				clearTimeout(ntpPageTimeout);
				ronn[target]();
			});
		});
		$(document).onSwipe(function(res){
			if(res.left)$('.offcanvas-collapse').addClass('open');
			else if(res.right)$('.offcanvas-collapse').removeClass('open');
		});
		$(document).on("click","#wifiAvailable li",function(){
			$("#ssid").val($("span",this).text());
			$('#wifiAvailable li').css("background-color","#FFF");
			$(this).css("background-color","#DDD");
		});
		$('#preload').remove();
		ronn.homePage();
	},
	
	/*
	 * PAGE
	 * *****************************************************************************
	 */
	getPage	: function(process,callback){
		$.ajax({
			url     : "/"+process,
			type	: "POST",
			dataType: "json",
			timeout	: 3000
		}).done(function(dt){
			$('.errBoard').remove();
			if(dt.success=="1"){
				if(typeof callback == 'function')callback(dt);
			}
			else {
				msg('error',dt.message);
				loading('hide');
				if(typeof callback == 'function')callback(false);
			}
		}).fail(function(m){
			msg('error',errBoard);
			loading('hide');
			if(typeof callback == 'function')callback(false);
		});
	},
	homePage	: function(){
		clearTimeout(homeTimeout);
		ronn.getPage('homePage',function(dt){
			if(dt){
				dt.data.other.lamp_status=='1'?
					$('.btn-lamp').addClass('btn-success').removeClass('btn-outline-danger'):
					$('.btn-lamp').addClass('btn-outline-danger').removeClass('btn-success');
				dt.data.other.pump_status=='1'?
					$('.btn-pump').addClass('btn-success').removeClass('btn-outline-danger'):
					$('.btn-pump').addClass('btn-outline-danger').removeClass('btn-success');
				$('#homePage .btn').btnMsg('unloading');
				delete dt.data.other;
				let view = "";
				$.each( dt.data, function( k, v ) {
					view += '<div class="card p-3 mt-2">';
					view += '<legend class="w-auto mt-0 mb-1 text-info">'+ k +'</legend>';
					view += '<div>';
					$.each( v, function( key, value ) {
						view += '<div class="form-group">';
						view += '<label>'+key+'</label>';
						view += '<div class="form-control">'+value+'</div>';
						view += '</div>';
					});
					view += '</div>';
					view += '</div>';
				});
				$('#homeContent').html(view);
				loading('hide');
			}
			if($('#homePage').is(":visible"))
				homeTimeout	= setTimeout(function(){ ronn.homePage(); }, 1000);
		});
	},
	lampPage	: function(){
		ronn.getPage('lampPage',function(dt){
			if(dt){
				$.each( dt.data, function( k, v ) {
					$('#'+k).val(v);
					$('#'+k+'Out').val(v);
				});
				loading('hide');
			}
		});
	},
	fishFeederPage	: function(){
		ronn.getPage('fishFeederPage',function(dt){
			if(dt){
				$.each( dt.data, function( k, v ) {
					$('#'+k).val(v);
					$('#'+k+'Out').val(v);
				});
				loading('hide');
			}
		});
	},
	ntpPage	: function(opt){
		ronn.getPage('ntpPage',function(dt){
			if(dt){
				if(opt=='getData'){//yang kedua dan selanjutnya hanya show waktu dan suhu
					delete dt.data.ntpServer;
					delete dt.data.timeZone;
				}
				else 
					dt.data.timeZone=Math.round(dt.data.timeZone*10)/10;// arduino always print 2 digit after point on float...
				$.each(dt.data, function( key, value ) {
					if(key.substring(0, 4)=='show')
						$('#'+key).html(value);
					else
						$('#'+key).val(value);
				});
				loading('hide');
			}
			if($('#ntpPage').is(":visible"))
				ntpPageTimeout	= setTimeout(function(){ ronn.ntpPage('getData'); }, 1000);
		});
	},
	wifiPage	: function(){
		ronn.getPage('wifiPage',function(dt){
			if(dt){
				$('#apName').html(dt.data.apName);
				$('#apPass').html(dt.data.apPass.length<1?'--no password--':dt.data.apPass);
				$('#currentConnection').val(dt.data.currentConnection);
				if(dt.data.wifiAvailable.length > 0){
					$('#wifiAvailable').html(dt.data.wifiAvailable);
					$('#wifiAvailable').parent().show();
				}
				else $('#wifiAvailable').parent().hide();
				loading('hide');
			}
		});
	},
	aboutPage	: function(){
		loading('hide');
	},
	
	
	/*
	 * OTHER FUNCTION
	 * *****************************************************************************
	 */
	saveForm	: function(self){
		let loc = $(self).closest('main').attr('id'); //location
		let prc = $(self).prev('legend').text().replace(/\s/g,''); //process
		let $btn= $(self).find(':submit');
		let dt = {};
		$(self).find('.form-control').each(function(){
			dt[$(this).attr("id")] 	= $(this).val(); 
		});
		$(self).find('output').each(function(){
			dt[$(this).attr("id")] 	= $(this).val(); 
		});
		console.log(loc);
		console.log(prc);
		console.log(dt);
		$btn.btnMsg('loading')
		$.ajax({
			url     : "/save"+prc,
			type	: "POST",
			data	: dt,
			dataType: "json"
		}).done(function(dt){
			if(dt.success=="1"){
				$btn.btnMsg('saved');
				msg('info',dt.message);
			}
			else{
				$btn.btnMsg('error');
				msg('error',dt.message);
			}
		}).fail(function(m){
			$btn.btnMsg('error');
			msg('error',m.status+" - "+m.statusText);
		});
		return false;
	},
	
	chButton	: function(self,cmd){
		clearTimeout(homeTimeout);
		stopMsg = true;
		$(self).btnMsg('loading');
		$.ajax({
			url     : "/btn"+cmd,
			type	: "POST",
			dataType: "json",
			timeout	: 11000 // max feed 10x ==> 10 second
		}).done(function(dt){
			stopMsg = false;
			$('.errBoard').remove();
			// dt.success=="1"?
				// msg('info',dt.message):
				// msg('error',dt.message);
			//$(self).btnMsg('unloading');
			ronn.homePage();
		}).fail(function(m){
			stopMsg = false;
			//msg('error',errBoard);
			$(self).btnMsg('unloading');
			ronn.homePage();
		});
	},
	forceUpdateNtp	: function(self){
		$(self).btnMsg('loading');
		$.ajax({
			url     : "/forceUpdateNtp",
			type	: "POST",
			dataType: "json"
		}).done(function(dt){
			if(dt.success=="1"){
				$(self).btnMsg('done');
				msg('info',dt.message);
			}
			else{
				$(self).btnMsg('error');
				msg('error',dt.message);
			}
		}).fail(function(m){
			$(self).btnMsg('error');
			msg('error',m.status+" - "+m.statusText);
		});
	},
	reConnect	: function(self){
		if(confirm('Confirm?')){
			$(self).btnMsg('loading');
			$.ajax({
				url     : "/reConnect",
				type	: "POST",
				dataType: "json"
			}).done(function(dt){
				if(dt.success=="1"){
					$(self).btnMsg('done');
					msg('info',dt.message);
				}
				else{
					$(self).btnMsg('error');
					msg('error',dt.message);
				}
			}).fail(function(m){
				$(self).btnMsg('error');
				msg('error',m.status+" - "+m.statusText);
			});
		}
	},
	rebootBoard	: function(self){
		if(confirm('Confirm?')){
			$(self).btnMsg('loading');
			$.ajax({
				url     : "/rebootBoard",
				type	: "POST",
				dataType: "json"
			}).done(function(dt){
				if(dt.success=="1"){
					$(self).btnMsg('done');
					msg('info',dt.message);
				}
				else{
					$(self).btnMsg('error');
					msg('error',dt.message);
				}
			}).fail(function(m){
				$(self).btnMsg('error');
				msg('error',m.status+" - "+m.statusText);
			});
		}
	},
	resetWifiSetting	: function(self){
		if(confirm('Confirm?')){
			$(self).btnMsg('loading');
			$.ajax({
				url     : "/resetWifiSetting",
				type	: "POST",
				dataType: "json"
			}).done(function(dt){
				if(dt.success=="1"){
					$(self).btnMsg('done');
					msg('info',dt.message);
				}
				else{
					$(self).btnMsg('error');
					msg('error',dt.message);
				}
			}).fail(function(){
				$(self).btnMsg('error');
				msg('error',m.status+" - "+m.statusText);
			});
		}
	},
	
	
	
	
	
	
}
$(function () {
	'use strict'
	ronn.init();
})




function loading(opt,callback){
	$('#loading').modal(opt);
	setTimeout(function () {
		if(typeof callback == 'function') { // make sure the callback is a function
			callback();
		}
	},100);
}

var msgTimer;
function msg(opt,msg){//("info"/"error", message)
	if(stopMsg) return;
	let tag	= opt=='error'?'<b>Error! </b>':'<b>Message! </b>';
	msg = msg==errBoard?errBoard:msg;
	$('.msg').remove();
	setTimeout(function(){
		$('.msg').remove();//karena delay 100, jika ada yang bareng tumpuk...
		msg==errBoard?
			$('body').append('<div class="msg errBoard '+opt+'" onclick="$(this).remove();clearTimeout(msgTimer);">'+tag+errBoard+'</div>'):
			$('body').append('<div class="msg '+opt+'" onclick="$(this).remove();clearTimeout(msgTimer);">'+tag+msg+'</div>');
		msgTimer = setTimeout(function(){$('.msg').slideUp();},3000);
	},100);	
}


$.fn.btnMsg = function(opt){
	var self = this;
	if(opt=="loading"){
		$(self).addClass('m-progress');
		$(self).attr('disabled',true);
	}
	else if(opt=="saved"){
		$(self).addClass('m-progress-saved');
		$(self).attr('disabled',true);
		setTimeout(function(){
			$(self).removeClass('m-progress m-progress-saved m-progress-error m-progress-done');
			$(self).removeAttr('disabled');
		},1500);
	}
	else if(opt=="done"){
		$(self).addClass('m-progress-done');
		$(self).attr('disabled',true);
		setTimeout(function(){
			$(self).removeClass('m-progress m-progress-saved m-progress-error m-progress-done');
			$(self).removeAttr('disabled');
		},1500);
	}
	else if(opt=="error"){
		$(self).addClass('m-progress-error');
		$(self).attr('disabled',true);
		setTimeout(function(){
			$(self).removeClass('m-progress m-progress-saved m-progress-error m-progress-done');
			$(self).removeAttr('disabled');
		},1500);
	}
	else{
		$(self).removeClass('m-progress m-progress-saved m-progress-error');
		$(self).removeAttr('disabled');
	}
};
