package com.virtualapplications.play;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

import com.virtualapplications.play.database.GameInfo;
import com.virtualapplications.play.database.GamesDbAPI;
import com.virtualapplications.play.database.SqliteHelper;

import org.w3c.dom.Document;

import java.io.File;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ExecutionException;

/**
 * Created by alawi on 8/4/15.
 */
public class CoverAdapter extends BaseAdapter {
    private final List<File> mImages;
    private Context mContext;

        public CoverAdapter(Context c,List<File> images) {
            mContext = c;
            mImages = images;
        }

    public int getCount() {
        //return mThumbIds.length;
        return mImages.size();
    }

    public Object getItem(int position) {
        return null;
    }

    public long getItemId(int position) {
        return position;
    }

    // create a new ImageView for each item referenced by the Adapter
    public View getView(int position, View convertView, ViewGroup parent) {
        ImageView imageView;
        if (convertView == null) {
            // if it's not recycled, initialize some attributes
            imageView = new ImageView(mContext);
            imageView.setLayoutParams(new GridView.LayoutParams(300, 400));
            imageView.setScaleType(ImageView.ScaleType.CENTER_CROP);
            imageView.setPadding(8, 8, 8, 8);
        } else {
            imageView = (ImageView) convertView;
        }
        GameInfo gameInfo = null;
        if (gameInfo == null) {
            gameInfo = new GameInfo(mContext);
        }
        String[] st = gameInfo.getGameInfo(mImages.get(position));

        if (st != null){
            Bitmap bt = gameInfo.getImage(st[0],st[3]);
            if (bt != null)
                imageView.setImageBitmap(bt);
        } else {
            imageView.setImageResource(R.drawable.boxart);
        }

        return imageView;
    }

}

