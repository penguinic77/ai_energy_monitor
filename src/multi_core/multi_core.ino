TaskHandle_t task1;
TaskHandle_t task2;
void codeForTask1( void * parameter )
{
for(;;) {
delay(1000);
Serial.print("This Task1 run on Core: ");
Serial.println(xPortGetCoreID());
delay(1000);
}
}
void codeForTask2( void * parameter )
{
for(;;) {
delay(1000);
Serial.print("This Task2 run on Core: ");
Serial.println(xPortGetCoreID());
delay(1000);
}
}
void setup() {
  Serial.begin(115200);
xTaskCreatePinnedToCore(
  codeForTask1, /*Task Function. */
  "Task1", /*name of task. */
  1000, /*Stack size of task. */
  NULL, /* parameter of the task. */
  1, /* proiority of the task. */
  &task1, /* Task handel to keep tra ck of created task. */
  0);
  vTaskDelay(100);
  
  xTaskCreatePinnedToCore(
  codeForTask2, /*Task Function. */
  "Task2", /*name of task. */
  1000, /*Stack size of task. */
  NULL, /* parameter of the task. */
  1, /* proiority of the task. */
  &task2, /* Task handel to keep tra ck of created task.就是前面宣告的第一二行，前面加& */
  1);
}

void loop() {
 Serial.print("This Loop Task run on Core: ");
 Serial.println(xPortGetCoreID());
 delay(1000);

}
