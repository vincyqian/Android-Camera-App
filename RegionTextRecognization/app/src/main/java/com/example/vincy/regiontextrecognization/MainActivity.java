package com.example.vincy.regiontextrecognization;


import android.Manifest;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.example.vincy.regiontextrecognization.zbar.CaptureActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.RequestBody;
import okhttp3.Response;


public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    private final MediaType MEDIA_TYPE_PNG = MediaType.parse("image/png");
    public final static int PERMISSION_CODE_FIRST = 0x13;//权限请求码
    private ScratchCardView mScratchCardView;
    private Bitmap BackGroundBitmap;
    private Bitmap cropBitmap,newBitmap;
    private String imagePath;
    private final static int SCANNIN_GREQUEST_CODE = 1;
    private final static int REQUEST_CAMERA=2;
    private File cameraSavePath;
    private Uri uri;
    private TextView textView;
    private int measuredWidth;
    private int measuredHeight;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        boolean checkPermissionFirst = PermissionUtils.checkPermissionFirst(this, PERMISSION_CODE_FIRST,
                new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.CAMERA});
        if (checkPermissionFirst) {
            init();
        }

    }
    private void init(){

        setContentView(R.layout.activity_main);
        mScratchCardView=findViewById(R.id.scratchCardView);
        mScratchCardView.post(new Runnable() {
                                  @Override
                public void run() {

                        measuredWidth = mScratchCardView.getMeasuredWidth();
                        measuredHeight = mScratchCardView.getMeasuredHeight();
                                  }
        });

        textView=findViewById(R.id.tvResult);
        mScratchCardView.setVisibility(View.INVISIBLE);
        initListener();

    }

    @Override
    protected void onStart(){
        super.onStart();
        mScratchCardView.post(new Runnable() {
            @Override
            public void run() {
                measuredWidth = mScratchCardView.getMeasuredWidth();
                measuredHeight = mScratchCardView.getMeasuredHeight();
            }
        });

    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        mScratchCardView.setVisibility(View.GONE);
        switch (requestCode) {
            case SCANNIN_GREQUEST_CODE:

                if (resultCode == RESULT_OK) {
                    String result = data.getStringExtra("QR_CODE");
                    // TODO 获取结果，做逻辑操作
                    textView.setText(result);
                } else {
                    Toast.makeText(this, "无法获取扫码结果", Toast.LENGTH_LONG ).show();
                }
                break;

            case REQUEST_CAMERA:
                if ( resultCode == RESULT_OK) {
                    Log.e("TAG", "---------" + FileProvider.getUriForFile(this, "com.example.vincy.regiontextrecognization.fileprovider", cameraSavePath));
                    mScratchCardView.setVisibility(View.VISIBLE);
                    findViewById(R.id.iv_camera_result_ok).setVisibility(View.VISIBLE);
                    findViewById(R.id.iv_camera_result_cancel).setVisibility(View.VISIBLE);
                    findViewById(R.id.tvResult).setVisibility(View.GONE);
                    findViewById(R.id.iv_camera).setVisibility(View.GONE);
                    findViewById(R.id.iv_barcode).setVisibility(View.GONE);

                    String photoPath;
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                        photoPath = String.valueOf(cameraSavePath);
                    } else {
                        photoPath = uri.getEncodedPath();
                    }
                    Log.d("拍照返回图片路径:", photoPath);

                    BitmapFactory.Options options = new BitmapFactory.Options();
                    TypedValue value=new TypedValue();
                    options.inTargetDensity = value.density;
                    options.inScaled = false;


                    File file = new File(photoPath);
                    if(file.exists()){
                        mScratchCardView.setVisibility(View.VISIBLE);
                        DisplayMetrics mDisplayMetrics = new DisplayMetrics();//屏幕分辨率容器
                        getWindowManager().getDefaultDisplay().getMetrics(mDisplayMetrics);
                        int densityDpi = mDisplayMetrics.densityDpi;
                        Bitmap temp=BitmapFactory.decodeFile(photoPath,options);
                        float radio1=measuredHeight/(float)temp.getHeight();
                        float radio2=measuredWidth/(float)temp.getWidth();
                        float radio=radio1<radio2?radio1:radio2;
                        Matrix matrix=new Matrix();
                        matrix.postScale(radio,radio);
                        BackGroundBitmap=Bitmap.createBitmap(temp,0,0,temp.getWidth(),temp.getHeight(),matrix,true);
                        BackGroundBitmap.setDensity(densityDpi);
                        mScratchCardView.setBackGroundBitmap(BackGroundBitmap);
                    }
                }


        }
    }


    private Bitmap scaleBitmap(Bitmap origin, float ratio) {
        if (origin == null) {
            return null;
        }
        int width = origin.getWidth();
        int height = origin.getHeight();
        Matrix matrix = new Matrix();
        matrix.preScale(ratio, ratio);
        Bitmap newBM = Bitmap.createBitmap(origin, 0, 0, width, height, matrix, false);
        if (newBM.equals(origin)) {
            return newBM;
        }
        origin.recycle();
        return newBM;
    }



    private void initListener() {
        findViewById(R.id.iv_camera_result_ok).setOnClickListener(this);
        findViewById(R.id.iv_camera_result_cancel).setOnClickListener(this);
        findViewById(R.id.iv_barcode).setOnClickListener(this);
        findViewById(R.id.iv_camera).setOnClickListener(this);
    }

    public void onClick(View v) {
        int id = v.getId();
        if (id == R.id.iv_camera_result_ok) {

            RectF rectF = new RectF();
            mScratchCardView.mPath.computeBounds(rectF,true);
            mScratchCardView.mPath.reset();
            if(rectF.centerX()==0&&rectF.centerX()==0)//PATH为空的情况
            {
                cropBitmap=BackGroundBitmap;
            }else{
                cropBitmap = Bitmap.createBitmap(BackGroundBitmap, (int) rectF.left, (int) rectF.top, (int) rectF.width(), (int) rectF.height());
            }//mScratchCardView.setBackGroundBitmap(cropBitmap);
            saveFile(cropBitmap);
            File file=new File(imagePath);
            sendPictureToServer(file);
            findViewById(R.id.iv_camera_result_ok).setVisibility(View.GONE);
            findViewById(R.id.iv_camera_result_cancel).setVisibility(View.GONE);
            findViewById(R.id.scratchCardView).setVisibility(View.GONE);
            findViewById(R.id.tvResult).setVisibility(View.VISIBLE);
            findViewById(R.id.iv_camera).setVisibility(View.VISIBLE);
            findViewById(R.id.iv_barcode).setVisibility(View.VISIBLE);

        } else if (id == R.id.iv_camera_result_cancel) {
            mScratchCardView.resetForeCanvas();
        }else if (id == R.id.iv_barcode) {

            Intent intent = new Intent();
            intent.setClass(MainActivity.this, CaptureActivity.class);
            startActivityForResult(intent, SCANNIN_GREQUEST_CODE);
        }
        else if (id == R.id.iv_camera) {
            findViewById(R.id.tvResult).setVisibility(View.GONE);
            Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
            cameraSavePath = new File(Environment.getExternalStorageDirectory().getAbsolutePath()
                    + "/test/"  + "temp.jpg");
            cameraSavePath.getParentFile().mkdirs();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                //第二个参数为 包名.fileprovider
                uri = FileProvider.getUriForFile(MainActivity.this, "com.example.vincy.regiontextrecognization.fileprovider", cameraSavePath);
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            } else {
                uri = Uri.fromFile(cameraSavePath);
            }
            intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
            startActivityForResult(intent, REQUEST_CAMERA);

        }
    }


    private void sendPictureToServer( File file) {

        //接口地址
        String urlAddress = "http://192.168.31.123:5000/ocr";
        if (file != null && file.exists()) {
            MultipartBody.Builder builder = new MultipartBody.Builder()
                   .setType(MediaType.parse("multipart/form-data;charset=utf-8"))
                   .addFormDataPart("files", "temp.jpg",
                           RequestBody.create(MEDIA_TYPE_PNG, file));
            HttpUtil.sendMultipart(urlAddress, builder.build(), new Callback() {
                @Override
                public void onFailure(Call call, IOException e) {
                    Log.d("---", "onFailure: ");
                    textView.setText("Fail to connet to server");

                }

                @Override
                public void onResponse(Call call, Response response) throws IOException {
                    Log.e("tag", response.protocol() + " " + response.code() + " " + response.message());
                    if (response.isSuccessful()) {
                        parseJsonWithJsonObject(response);
                    }
                }
            }
            );
        }
    }
    private void parseJsonWithJsonObject(Response response) throws IOException {
//        Toast.makeText(MainActivity.this, "服务器连接成功", Toast.LENGTH_SHORT).show();
        newBitmap=Bitmap.createBitmap(cropBitmap.getWidth(),cropBitmap.getHeight(),cropBitmap.getConfig());
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
            String textAll="";
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
                TextView textView=findViewById(R.id.tvResult);
                textAll=textAll+jsonObject.getString("text");

            }
            textView.setText(textAll);

        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
    public void saveFile(Bitmap bitmap) {//将Bitmap类型的图片转化成file类型，便于上传到服务器
        /*保存图片到sdcard并返回图片路径*/
        if (FileUtils.createOrExistsDir(Constant.DIR_ROOT)) {
            StringBuffer buffer = new StringBuffer();
            imagePath = "";
            imagePath = buffer.append(Constant.DIR_ROOT).append("temp.jpg").toString();
            ImageUtils.save(bitmap, imagePath, Bitmap.CompressFormat.JPEG);
        }

    }

}