$(document).ready(function(){
    // if desktop viewport then footer at the bottom
    if($(window).width() > 991){
        var height = $(document).height() - $('body').height();
        $('body').css('margin-bottom', height);
    }
});