package com.example.whatsappclone;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;


import com.example.whatsappclone.Chat.MessageAdapter;
import com.example.whatsappclone.Chat.ChatObject;
import com.example.whatsappclone.Chat.MessageObject;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.ChildEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;


public class ChatActivity extends AppCompatActivity {

    private RecyclerView mChat;
    private RecyclerView.Adapter mChatAdaptter;
    private RecyclerView.LayoutManager mChatLayoutManager;
    String chatID;
    DatabaseReference mChatDb;

    ArrayList<MessageObject> messageList;

    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat);

        chatID = getIntent().getExtras().getString("chatID"); //ChatListAdapter에서 저장한 chatID를 읽어옴

        mChatDb =  FirebaseDatabase.getInstance().getReference().child("chat").child("chatID");

        Button mSend = findViewById(R.id.send);
        mSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage();
            }
        });
        initializeRecyclerView();
        getChatMessages();
    }

    private void getChatMessages() {
        mChatDb.addChildEventListener(new ChildEventListener() {
            @Override
            public void onChildAdded(@NonNull DataSnapshot dataSnapshot, @Nullable String s) {
                if(dataSnapshot.exists()){
                    String  text = "",
                            creatorId = "";


                    if(dataSnapshot.child("text").getValue() != null){
                        text = dataSnapshot.child("text").getValue().toString();
                    }

                    if(dataSnapshot.child("creator").getValue() != null){
                        creatorId = dataSnapshot.child("creator").getValue().toString();
                    }

                    MessageObject mMessage = new MessageObject(dataSnapshot.getKey(),creatorId,text);
                    messageList.add(mMessage);
                    mChatLayoutManager.scrollToPosition(messageList.size()-1); //가장 최근의 메세지로 자동 움직임
                    mChatAdaptter.notifyDataSetChanged();
                }
            }

            @Override
            public void onChildChanged(@NonNull DataSnapshot dataSnapshot, @Nullable String s) {}

            @Override
            public void onChildRemoved(@NonNull DataSnapshot dataSnapshot) {}

            @Override
            public void onChildMoved(@NonNull DataSnapshot dataSnapshot, @Nullable String s) {}

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {}
        });
    }

    private void sendMessage(){
        EditText mMessage = findViewById(R.id.messageEdit);

        if(!mMessage.getText().toString().isEmpty()){ //Let's save message!
            DatabaseReference newMessageDB = mChatDb.push();

            Map newMesageMap = new HashMap<>();
            newMesageMap.put("text", mMessage.getText().toString());
            newMesageMap.put("creator", FirebaseAuth.getInstance().getUid());

            newMessageDB.updateChildren(newMesageMap);
        }
        mMessage.setText(null);
    }

    private void initializeRecyclerView() {
        messageList = new ArrayList<>();
        mChat = findViewById(R.id.messageList);
        mChat.setNestedScrollingEnabled(false); //스크롤안보이게 하기
        mChat.setHasFixedSize(false);
        mChatLayoutManager = new LinearLayoutManager(getApplicationContext(), LinearLayout.VERTICAL,false); //UserList에 뭔가 특별한 일을 해주고 싶을 때 사용함
        mChat.setLayoutManager(mChatLayoutManager); //Manager 연결해주기
        mChatAdaptter = new MessageAdapter(messageList);
        mChat.setAdapter(mChatAdaptter);

    }
}
