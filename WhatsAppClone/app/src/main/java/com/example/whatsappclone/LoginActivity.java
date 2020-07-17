package com.example.whatsappclone;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseException;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.PhoneAuthCredential;
import com.google.firebase.auth.PhoneAuthProvider;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

public class LoginActivity extends AppCompatActivity {

    private EditText mPhoneNumber, mCode;
    private Button mSend;

    private PhoneAuthProvider.OnVerificationStateChangedCallbacks mCallbacks;

    String mVerificationId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        FirebaseApp.initializeApp(this); //Firebase code 사용 설정

        userIsLoggedIn(); //Check if User is Logged in

        mPhoneNumber = findViewById(R.id.phoneNumber);
        mCode = findViewById(R.id.code);

        mSend = findViewById(R.id.send);
        mSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mVerificationId != null){
                    verifyPhoneNumberWithCode();
                }else{
                    startPhoneNumberVerification();
                }
            }
        });

        mCallbacks = new PhoneAuthProvider.OnVerificationStateChangedCallbacks() {
            @Override
            public void onVerificationCompleted(PhoneAuthCredential phoneAuthCredential) { //if everything goes smoothly, this method will be ran
                singInWithPhoneAuthCredential(phoneAuthCredential);
            }

            @Override
            public void onVerificationFailed(FirebaseException e) { //Verification went wrong, this will happen
            }

            @Override
            public void onCodeSent(String Verification, PhoneAuthProvider.ForceResendingToken forceResendingToken){ //Code가 User에게 보내지면 Call 됨
                super.onCodeSent(Verification,forceResendingToken);
                mVerificationId = Verification;
                mSend.setText("VerifyCode");
            }
        };
    }

    private void verifyPhoneNumberWithCode(){
        PhoneAuthCredential credential = PhoneAuthProvider.getCredential(mVerificationId, mCode.getText().toString());
        singInWithPhoneAuthCredential(credential);
    }



    private void singInWithPhoneAuthCredential(PhoneAuthCredential phoneAuthCredential) {
        //signing with something like email, phone.. in this case, with phone number
        FirebaseAuth.getInstance().signInWithCredential(phoneAuthCredential).addOnCompleteListener(this, new OnCompleteListener<AuthResult>() {
            @Override
            public void onComplete(@NonNull Task<AuthResult> task) { //addOnCompleteListen : to know it is success or not
                if(task.isSuccessful()){
                    final FirebaseUser user = FirebaseAuth.getInstance().getCurrentUser(); //authentication page에 있는 정보
                    if(user != null){
                        final DatabaseReference mUserDB = FirebaseDatabase.getInstance().getReference().child("user").child(user.getUid()); //database의 값을 가리킴
                        mUserDB.addListenerForSingleValueEvent(new ValueEventListener() {
                            @Override
                            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                                if(!dataSnapshot.exists()){//data에 뭔가 없을 때, 정보들을 집어 넣어야 함
                                    Map<String, Object> userMap = new HashMap<>();
                                    userMap.put("phone",user.getPhoneNumber());
                                    userMap.put("name",user.getPhoneNumber());
                                    mUserDB.updateChildren(userMap); //DB에 데이터 넣기
                                }
                                userIsLoggedIn();
                            }

                            @Override
                            public void onCancelled(@NonNull DatabaseError databaseError) {

                            }
                        }); //database를 한번만 참고하고 말것이기 때문에, singleValueEvent


                    }
                }
                //userIsLoggedIn();
            }
        });

    }

    private void userIsLoggedIn() { //User가 Logged in 했으니까, 다음 Level로 넘어가기
        FirebaseUser user = FirebaseAuth.getInstance().getCurrentUser();
        if(user != null){ //Real User이 Logged in 했는지 Double Check
            startActivity(new Intent(getApplicationContext(),MainPageActivity.class));
            finish();
            return;
        }
    }

    private void startPhoneNumberVerification() {
        PhoneAuthProvider.getInstance().verifyPhoneNumber(
                mPhoneNumber.getText().toString(), //입력받은 폰번호
                60, //Timeout
                TimeUnit.SECONDS,
                this,
                mCallbacks);
    }
}
