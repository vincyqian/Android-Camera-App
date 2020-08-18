package com.example.vincy.opencvdemo;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Toast;

import com.wildma.idcardcamera.camera.CameraActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import okhttp3.Cache;
import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.Headers;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

/**
 * Author   wildma
 * Github   https://github.com/wildma
 * Date     2018/6/24
 * Desc     ${身份证相机使用例子}
 */


public class MainActivity extends AppCompatActivity {
    private ImageView imageView;
    private final MediaType MEDIA_TYPE_PNG = MediaType.parse("image/png");

    private static final String TAG = "MainActivity";
    private Bitmap newBitmap;
    private String picPath;
    private String ServerUrl="http://192.168.1.222:5000/ocr";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = (ImageView) findViewById(R.id.iv_image);
    }

    /**
     * 身份证正面
     */
    public void frontIdCard(View view) {
        CameraActivity.toCameraActivity(this, CameraActivity.TYPE_IDCARD_FRONT);
    }

    public void upload(View view) {
        HttpContact(picPath,"0");
    }

    public void UrlSure(View view) {
        EditText editText =(EditText)findViewById(R.id.UrlEdit);
        ServerUrl=editText.getText().toString();
    }

    public void preupload(View view) {
        HttpContact(picPath,"1");
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == CameraActivity.REQUEST_CODE && resultCode == CameraActivity.RESULT_CODE) {
            //获取图片路径，显示图片
            final String path = CameraActivity.getImagePath(data);
            if (!TextUtils.isEmpty(path)) {
                File file = new File(path);
                Bitmap bm=BitmapFactory.decodeFile(path);
                newBitmap=Bitmap.createBitmap(bm.getWidth(),bm.getHeight(),bm.getConfig());
                imageView.setImageBitmap(bm);
                picPath=path;
            }
        }
    }


    private void HttpContact(String path,String flag) {
        File file = new File(path);
        OkHttpClient okHttpClient = new OkHttpClient();

        MultipartBody requestBody = new MultipartBody.Builder()
                .setType(MultipartBody.FORM)
                .addFormDataPart("file", "idCardFrontCrop.jpg",
                        RequestBody.create(MediaType.parse("image/jpg"), file))
                .addFormDataPart("flag" , flag)
                .build();


        Request request = new Request.Builder()
                .url(ServerUrl)
                .post(requestBody)
                .build();

        okHttpClient.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(MainActivity.this, "服务器错误", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                });

                Log.d(TAG, "onFailure: " + e.getMessage());

            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                Log.d(TAG, response.protocol() + " " + response.code() + " " + response.message());

                if (response.isSuccessful()) {
                    parseJsonWithJsonObject(response);
                }
            }
        });


    }

    private void parseJsonWithJsonObject(Response response) throws IOException {
//        Toast.makeText(MainActivity.this, "服务器连接成功", Toast.LENGTH_SHORT).show();
        newBitmap=Bitmap.createBitmap(newBitmap.getWidth(),newBitmap.getHeight(),newBitmap.getConfig());
        Canvas canvas=new Canvas(newBitmap);
        Paint paint = new Paint();
        paint.setColor(Color.parseColor("#000000"));
//        paint.setTextSize(18);
        paint.setAntiAlias(true);//抗锯齿

        String responseData = response.body().string();
        try {

            JSONObject jsonobject = new JSONObject(responseData);
            double time = jsonobject.getDouble("timeTake");
            JSONArray jsonArray = jsonobject.getJSONArray("res");
            List<String> text = new ArrayList<>();
            List<Integer> w = new ArrayList<>();
            List<Integer> h = new ArrayList<>();
            List<Integer> cx = new ArrayList<>();
            List<Integer> cy = new ArrayList<>();
            for (int i = 0; i < jsonArray.length(); i++) {
                JSONObject jsonObject = jsonArray.getJSONObject(i);
                w.add(jsonObject.getInt("w"));
                h.add(jsonObject.getInt("h"));
                cx.add(jsonObject.getInt("cx"));
                cy.add(jsonObject.getInt("cy"));
                text.add(jsonObject.getString("text"));
                int ww=jsonObject.getInt("w");
                int hh=jsonObject.getInt("h");
                int cxcx=jsonObject.getInt("cx");
                int cycy=jsonObject.getInt("cy");
                paint.setTextSize(h.get(i)*4/5);
                canvas.drawText(jsonObject.getString("text"),(float)(cxcx-0.4*ww),cycy,paint);
            }
            imageView.setImageBitmap(newBitmap);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void downlodefile(Response response, String url, String fileName) {
        InputStream is = null;
        byte[] buf = new byte[2048];
        int len = 0;
        FileOutputStream fos = null;
        try {
            is = response.body().byteStream();
            //文件大小

            Bitmap bitmap = BitmapFactory.decodeStream(is);

            long total = response.body().contentLength();

            File file = new File(url, fileName);
            fos = new FileOutputStream(file);
            long sum = 0;
            while ((len = is.read(buf)) != -1) {
                fos.write(buf, 0, len);
//                进度条
//                sum += len;
//                int progress = (int) (sum * 1.0f / total * 100);
            }
            fos.flush();
            Log.e("xxxxxxxx", "下载成功");
        } catch (Exception e) {
        } finally {
            try {
                if (is != null)
                    is.close();
            } catch (IOException e) {
            }
            try {
                if (fos != null)
                    fos.close();
            } catch (IOException e) {
            }
        }
    }


}

