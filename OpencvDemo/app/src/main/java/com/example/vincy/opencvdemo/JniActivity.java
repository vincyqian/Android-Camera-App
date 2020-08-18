package com.example.vincy.opencvdemo;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ImageView;

public class JniActivity extends AppCompatActivity implements View.OnClickListener {
    private ImageView imageView;

    static {
        System.loadLibrary("-libnative");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_jni);
        imageView = findViewById(R.id.imageView);
        findViewById(R.id.show).setOnClickListener(this);
        findViewById(R.id.process).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.show) {
            Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test);
            imageView.setImageBitmap(bitmap);
        } else {
            Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test);
            imageView.setImageBitmap(bitmap);
        }
    }

}

