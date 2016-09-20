package com.github.yongzhizhan.spring.controller;

import com.github.yongzhizhan.spring.model.Capture;
import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import javax.servlet.http.HttpServletResponse;
import java.io.*;

@Controller
@SuppressWarnings("UnusedDeclaration")
public class IndexController {

    @Value("${example.message}")
    private String message;

    @RequestMapping(value = "/", method = RequestMethod.GET)
    @ResponseBody
    public String showIndex() {
        return message;
    }

    @RequestMapping(value = "/capture", method = RequestMethod.GET)
    @ResponseBody
    public void showImage(@RequestParam("name") String vName, HttpServletResponse vResponse) throws IOException {
        int tSize = 256;
        Pointer tPath = new Memory(tSize);
        Capture.INSTANCE.getCaptureImage("C:\\Users\\AtomView\\Desktop\\" + vName, tPath, tSize);
        String tFilePath = tPath.getString(0);

        vResponse.setContentType("image/jpeg"); // 设置返回内容格式
        File file = new File(tFilePath);       //括号里参数为文件图片路径
        if (file.exists()) {
            InputStream in = new FileInputStream(tFilePath);   //用该文件创建一个输入流
            OutputStream os = vResponse.getOutputStream();  //创建输出流
            byte[] b = new byte[1024];
            while (in.read(b) != -1) {
                os.write(b);
            }
            in.close();
            os.flush();
            os.close();
        }
    }
}
